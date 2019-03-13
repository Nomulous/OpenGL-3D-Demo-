#ifndef __geom_h__
#define __geom_h__

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <cmath>
#include <iostream>
#include <stdlib.h>

#include "entity.h"
#include "ObjMesh.h"
#include "misc.h"

static float getDim(float t, float d0, float d1, float d2, float d3) {
  float nt = 1 - t;
  return pow(nt, 3) * d0 + pow(nt, 2) * d1 * 3 * t + nt * d2 * 3 * pow(t, 2) + pow(t, 3) * d3;
}

static VBOList loadObject(ObjectCreator& oc, const char* file_path) {

    // Load the obj
    std::cout << "[geom.h:loadObject]: Loading OBJ file '" << file_path << "'..." << std::endl;
    ObjMesh mesh;
    mesh.load(file_path, false, false);

    // Get the data in a usable format
    int n = mesh.getNumIndexedVertices();
    glm::vec3* vert_coords = mesh.getIndexedPositions();
    glm::vec2* tex_coords = mesh.getIndexedTextureCoords();
    glm::vec3* normal_vecs = mesh.getIndexedNormals();
    GLfloat* points = new GLfloat[n * 3];
    GLfloat* uvs = new GLfloat[n * 2];
    GLfloat* normals = new GLfloat[n * 3];
    for (int i = 0; i < n; i ++) {
        int i2 = i * 2, i3 = i * 3;
        points[i3] = vert_coords[i].x;
        points[i3 + 1] = vert_coords[i].y;
        points[i3 + 2] = vert_coords[i].z;
        uvs[i2] = tex_coords[i].x;
        uvs[i2 + 1] = tex_coords[i].y;
        normals[i3] = normal_vecs[i].x;
        normals[i3 + 1] = normal_vecs[i].y;
        normals[i3 + 2] = normal_vecs[i].z;
    }

    // Create the object
    VBOList vbos;
    vbos.indices_vbo = oc.bindIndicesBuffer(mesh.getTriangleIndices(), mesh.getNumTriangles() * 3);
    vbos.positions_vbo = oc.storeInVBO(0, 3, points, n * 3);
    vbos.normals_vbo = oc.storeInVBO(1, 3, normals, n * 3);
    vbos.uv_vbo = oc.storeInVBO(2, 2, uvs, n * 2);
    vbos.vertices = n;
    delete[] points;
    delete[] uvs;
    delete[] normals;
    std::cout << "[geom.h:loadObject]: Done loading OBJ file." << std::endl;

    return vbos;
}

static VBOList createTerrain(ObjectCreator& oc, float width, float length,
    float max_height, float min_height) {

    float X_RESOLUTION = 10.0f;
    float Z_RESOLUTION = 10.0f;
    int MAX_Y_DIFF = 3;
    int PHASE_2_GEOM_LEVEL = 2;

    // Create the phase 1 data
    float mid = (max_height - min_height) / 2;
    int xn = static_cast<int>(width / X_RESOLUTION) + 2;
    int zn = static_cast<int>(length / Z_RESOLUTION) + 2;
    int rwidth = (xn * PHASE_2_GEOM_LEVEL) - PHASE_2_GEOM_LEVEL + 1;
    int cwidth = (zn * PHASE_2_GEOM_LEVEL) - PHASE_2_GEOM_LEVEL + 1;
    int n = rwidth * cwidth;
    GLfloat* points = new GLfloat[n * 3];
    GLfloat* normals = new GLfloat[n * 3];
    GLfloat* uvs = new GLfloat[n * 2];
    for (int x = 0; x < xn; x ++) {
        for (int z = 0; z < zn; z ++) {

            // Calculate the indices
            int current = (x * PHASE_2_GEOM_LEVEL) + z * rwidth;
            int left = (x == 0)? -1 : ((x - 1) * PHASE_2_GEOM_LEVEL) + z * rwidth;
            int above = (z == 0)? -1 : (x * PHASE_2_GEOM_LEVEL) + (z - 1) * rwidth;

            // Calculate the y coordinate
            float ly = (left < 0)? mid : points[left * 3 + 1];
            float ay = (above < 0)? mid : points[above * 3 + 1];
            float y1 = ly > ay? ay : ly, y2 = ly > ay? ly : ay;
            int diff = rand() % MAX_Y_DIFF;
            float y = 0;
            if (rand() % 2 == 0) { // go higher
                if (y2 + diff > max_height) {
                    y = max_height;
                } else {
                    y = y2 + diff;
                }
            } else { // go lower
                if (y1 - diff < min_height) {
                    y = min_height;
                } else {
                    y = y1 - diff;
                }
            }
            points[current * 3 + 1] = y;
        }
    }

    // Generate the phase 2 data
    for (int x = 0; x < rwidth; x ++) {
        for (int z = 0; z < cwidth; z ++) {

            // Calculate the x/z coordinate
            int current = x + z * rwidth;
            int right = (x >= rwidth - 1)? -1 : (x + 1) + z * rwidth;
            int left = (x == 0)? -1 : (x - 1) + z * rwidth;
            int above = (z == 0)? -1 : x + (z - 1) * rwidth;
            int below = (z >= cwidth - 1)? -1 : x + (z + 1) * rwidth;
            points[current * 3] = (width / rwidth) * x - width / 2;
            points[current * 3 + 2] = (length / cwidth) * z - length / 2;

            // Calculate all the y coordinates for the phase 2 points
            // Uses bezier curves to calculate intermidiate y values
            float y1 = points[current * 3 + 1];
            if (x % PHASE_2_GEOM_LEVEL == 0 && x < rwidth - PHASE_2_GEOM_LEVEL) {
                float y0 = (x - PHASE_2_GEOM_LEVEL < 0)? mid : points[(current - PHASE_2_GEOM_LEVEL) * 3 + 1];
                float y2 = (x + PHASE_2_GEOM_LEVEL >= rwidth - 1)? mid : points[(current + PHASE_2_GEOM_LEVEL) * 3 + 1];
                float y3 = (x + PHASE_2_GEOM_LEVEL * 2 >= rwidth - 1)? mid : points[(current + PHASE_2_GEOM_LEVEL * 2) * 3 + 1];
                for (int p = 1; p < PHASE_2_GEOM_LEVEL; p ++) {

                    // Determine the y coordinate of the intermidiate point
                    int idx = (current + p) * 3 + 1;
                    points[idx] = getDim(0.33333f + ((float) p / PHASE_2_GEOM_LEVEL) * 0.333333f,
                        y0, y1, y2, y3);
                }
            }

            // Calculate the UV coordinates
            uvs[current * 2] = ((float) x) / rwidth;
            uvs[current * 2 + 1] = ((float) z) / cwidth;

            // Calculate the normal
            float ly = (left < 0)? mid : points[left * 3 + 1];
            float ry = (right < 0)? mid : points[right * 3 + 1];
            float ay = (above < 0)? mid : points[above * 3 + 1];
            float by = (below < 0)? mid : points[below * 3 + 1];
            glm::vec3 normal = glm::normalize(glm::vec3(ly - ry, 2, by - ay));
            current *= 3;
            normals[current] = normal.x;
            normals[current + 1] = normal.y;
            normals[current + 2] = normal.z;
        }
    }

    // Calculate the indices
    int triangles_per_row = (rwidth - 1) * 2;
    int triangles = triangles_per_row * (cwidth - 1);
    int h = cwidth - 1;
    GLuint* indices = new GLuint[triangles * 3];
    for (int row = 0; row < h; row ++) {
        for (int col = 0; col < rwidth - 1; col ++) {

            int il = (row * triangles_per_row + col * 2) * 3;
            int tlIndex = col + row * cwidth;
            int trIndex = tlIndex + 1;
            int blIndex = tlIndex + cwidth;
            int brIndex = blIndex + 1;

            // Top left triangle
            indices[il++] = blIndex;
            indices[il++] = tlIndex;
            indices[il++] = trIndex;

            // Bottom right triangle
            indices[il++] = blIndex;
            indices[il++] = trIndex;
            indices[il] = brIndex;
        }
    }

    // Create the object
    VBOList terrain;
    terrain.indices_vbo = oc.bindIndicesBuffer(indices, triangles * 3);
    terrain.positions_vbo = oc.storeInVBO(0, 3, points, n * 3);
    terrain.normals_vbo = oc.storeInVBO(1, 3, normals, n * 3);
    terrain.uv_vbo = oc.storeInVBO(2, 2, uvs, n * 2);
    terrain.vertices = n * 3;
    delete[] indices;
    delete[] points;
    delete[] normals;
    delete[] uvs;

    return terrain;
}

static VBOList getCubeMesh(ObjectCreator& oc, float length) {

	// Determine vertices
	float w = length / 2;
	GLfloat vertices[] = {
			-w, w, w, -w, -w, w, w, -w, w, w, w, w,
			w, w, -w, w, -w, -w, -w, -w, -w, -w, w, -w,
			w, w, w, w, -w, w, w, -w, -w, w, w, -w,
			-w, w, -w, -w, -w, -w, -w, -w, w, -w, w, w,
			-w, w, -w, -w, w, w, w, w, w, w, w, -w,
			-w, -w, w, -w, -w, -w, w, -w, -w, w, -w, w
	};

	// Calculate normals, indices, and texture coordinates
	GLfloat normals[72];
	GLuint indices[36];
	GLfloat textureCoords[48];
    for (int i = 0; i < 72; i ++) {
        normals[i] = 0.0f;
    }
	for (int i = 0; i < 6; i ++) {
		int nl = i * 12, il = i * 6, tl = i * 8;

		// Normals
		int offset = ((i / 2) + 2) % 3;
		int num = (i % 2 == 0)? 1 : -1;
		for (int j = 0; j < 4; j ++) {
			normals[nl + offset + j * 3] = num;
		}

		// Indices
		num = i * 4;
		indices[il] = num;
		indices[il + 1] = num + 1;
		indices[il + 2] = num + 3;
		indices[il + 3] = num + 3;
		indices[il + 4] = num + 1;
		indices[il + 5] = num + 2;

		// Texture coordinates
		textureCoords[tl + 3] = 1;
		textureCoords[tl + 4] = 1;
		textureCoords[tl + 5] = 1;
		textureCoords[tl + 6] = 1;
	}

    VBOList cube;
    int n = 24;
    cube.indices_vbo = oc.bindIndicesBuffer(indices, 36);
    cube.positions_vbo = oc.storeInVBO(0, 3, vertices, n * 3);
    cube.normals_vbo = oc.storeInVBO(1, 3, normals, n * 3);
    cube.uv_vbo = oc.storeInVBO(2, 2, textureCoords, n * 2);
    cube.vertices = 36;

    return cube;
}

#endif
