/* Project: CSCI 4110U Final Project (F2018)
 * Authors: Betty Kwong, Devon McGrath, Martin Tuzim
 */

#define GLM_ENABLE_EXPERIMENTAL

#include <GL/glew.h>
#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include <iostream>
#include <list>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>

#include "entity.h"
#include "geom.h"
#include "misc.h"

#define TITLE "CSCI 4110U - Final Project"

#define PI 3.14159265358979323846264

#define TIME_FACTOR 1.0f

// Directories
#define DIR_RES "./res/"
#define DIR_SHADERS DIR_RES"shaders/"
#define DIR_MESHES DIR_RES"meshes/"

// Camera modes
#define CAMERA_FREE 1

// Special keys
#define SP_KEY_LEFT 0
#define SP_KEY_UP 1
#define SP_KEY_RIGHT 2
#define SP_KEY_DOWN 3

// Object constants
#define TYPE_PLANE 7657
#define PLANE1_ID 1891

/*---------------------------- Variables ----------------------------*/

bool* key_states = new bool[256];
bool* special_keys = new bool[5];

glm::vec3 background = glm::vec3(0.5294f, 0.8078f, 0.98039f);

// Window variables
int width = 1280;
int height = 720;
int window_id = 0;

// Shader programs
int basic_prog_ID = 0;

Camera camera;
int view_mode = CAMERA_FREE;
glm::mat4 projection_mat;
glm::mat4 view_mat;

glm::vec3 sun_dir = glm::vec3(0, -0.196116f, -0.98058f);

std::vector<Entity> basic_entities;

// Time variables
float delta_seconds = 0.0f, elapsed = 0.0f;

ObjectCreator oc;

// Plane Data
GLuint vertexBuffer[2];
GLuint indexBuffer[2];
GLenum positionBufferId[2];
GLuint positions_vbo[2] = {0, 1};
GLuint textureCoords_vbo[2] = {0, 1};
GLuint normals_vbo[2] = {0, 1};
GLuint colours_vbo[2] = {0, 1};
unsigned int numVertices[2];

glm::mat4 translationMatrixPlaneOne;
glm::mat4 translationMatrixPlaneTwo;

/*---------------------------- Functions ----------------------------*/

GLuint loadShader(const GLenum& type, const char* file);
int createProgram(const char* vertex_file, const char* frag_file,
	std::list<std::string> attribs);
void init();

/* Creates a view matrix looking at a specific point.  */
glm::mat4 getViewMatrix(glm::vec3 pos, glm::vec3 target) {
	return inverse(lookAt(pos, target, glm::vec3(0, 1, 0)));
}

/* Creates a view matrix with a specific rotation around a point. */
glm::mat4 getViewMatrix(glm::vec3 pos, float pitch, float yaw, float roll) {

	glm::mat4 identity = glm::mat4(1.0f);
	glm::mat4 translation = glm::translate(identity, glm::vec3(-pos.x, -pos.y, -pos.z));
	glm::mat4 rotation = glm::rotate(identity, pitch, glm::vec3(1, 0, 0));
	rotation = glm::rotate(rotation, yaw, glm::vec3(0, 1, 0));
	rotation = glm::rotate(rotation, roll, glm::vec3(0, 0, 1));

	return rotation * translation;
}

/* Creates a view matrix with a specific rotation around a point. */
glm::mat4 getViewMatrix(Camera cam) {
	return getViewMatrix(glm::vec3(cam.x, cam.y, cam.z),
		cam.pitch, cam.yaw, cam.roll);
}

/* Creates a perspective projection matrix. */
static glm::mat4 getProjectionMatrix() {
	float aspectRatio = (float) width / (float) height;
	float FOV = 45.0f, Z_NEAR = 0.1f, Z_FAR = 1000.0f;
	return glm::perspective(glm::radians(FOV), aspectRatio, Z_NEAR, Z_FAR);
}

/* Handles a window resize by updating the projection matrix in the shaders. */
static void reshape(int w, int h) {

	// Update the view
    glViewport(0, 0, w, h);
    width = w;
    height = h;
	projection_mat = getProjectionMatrix();
}

static void drag(int x, int y) {

}

static void mouse(int button, int state, int x, int y) {

}

static void keyboardUp(unsigned char key, int x, int y) {
	key_states[key] = false;
}

static void keyboard(unsigned char key, int x, int y) {

	// Handle a quit (user pressed ESC)
	if (key == 27) {

		// Stop drawing the UI
		glutDestroyWindow(window_id);

		// Clean up the memory
		oc.destroy();
		delete[] key_states;
		delete[] special_keys;


		// Exit the program
		std::cout << "Program exiting (user pressed ESC)." << std::endl;
		exit(0);
	}

	key_states[key] = true;

	std::cout << "Key pressed: '" << key << "' (int value: " <<
		static_cast<int>(key) << ")" << std::endl;
}

static void render(void) {

	// Prepare to render
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(background.x, background.y, background.z, 1);

	if (CAMERA_FREE) {
		view_mat = getViewMatrix(camera);
	} else {
		// TODO
		view_mat = getViewMatrix(glm::vec3(0.0f, 50.0f, -125.0f), glm::vec3(-250.0f, 25.0f, -125.0f));
	}

	// Setup the basic shader program
	glUseProgram(basic_prog_ID);
	int transformation_mat_loc = glGetUniformLocation(basic_prog_ID, "transformation_mat");
	int projection_mat_loc = glGetUniformLocation(basic_prog_ID, "projection_mat");
	glUniformMatrix4fv(projection_mat_loc, 1, GL_FALSE, &projection_mat[0][0]);
	int view_mat_loc = glGetUniformLocation(basic_prog_ID, "view_mat");
	glUniformMatrix4fv(view_mat_loc, 1, GL_FALSE, &view_mat[0][0]);
	int camera_loc = glGetUniformLocation(basic_prog_ID, "camera");
	glUniform3fv(camera_loc, 1, (GLfloat*)&camera.getPosition()[0]);
	int light_dir_loc = glGetUniformLocation(basic_prog_ID, "light_dir");
	glUniform3fv(light_dir_loc, 1, (GLfloat*)&sun_dir[0]);
	int colour_loc = glGetUniformLocation(basic_prog_ID, "colour");

	// Render all the entities with the basic shader program
	for (auto e : basic_entities) {

        // Make sure it is visible
        if (!e.visible) {
            continue;
        }

		// Upload the entity-specific values
		glUniformMatrix4fv(transformation_mat_loc, 1, GL_FALSE,
			&e.GetTransformationMatrix()[0][0]);
		glUniform3fv(colour_loc, 1, (GLfloat*)&e.colour[0]);

		// Send the data to the shaders
		glBindBuffer(GL_ARRAY_BUFFER, e.vbos.positions_vbo);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
		glBindBuffer(GL_ARRAY_BUFFER, e.vbos.normals_vbo);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// Send the indices to the shaders
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, e.vbos.indices_vbo);

		// Draw the entity
		glDrawElements(GL_TRIANGLES, e.vbos.vertices, GL_UNSIGNED_INT, (void*)0);
	}

	//Draw one plane
	{
		glm::vec3 myColour = glm::vec3(0.0f, 0.0f, 1.0f);
		glm::mat4 modelMat =  translationMatrixPlaneOne;

		// Upload the entity-specific values
		glUniformMatrix4fv(transformation_mat_loc, 1, GL_FALSE,
			&modelMat[0][0]);
		glUniform3fv(colour_loc, 1, (GLfloat*)&myColour[0]);

		// find the names (ids) of each vertex attribute
  		GLint positionAttribId = glGetAttribLocation(basic_prog_ID, "position");
 		GLint textureCoordsAttribId = glGetAttribLocation(basic_prog_ID, "textureCoords");
  		GLint normalAttribId = glGetAttribLocation(basic_prog_ID, "normal");

		// provide the vertex positions to the shaders
  		glBindBuffer(GL_ARRAY_BUFFER, positions_vbo[0]);
  		glEnableVertexAttribArray(positionAttribId);
		glVertexAttribPointer(positionAttribId, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

		// provide the vertex texture coordinates to the shaders
		glBindBuffer(GL_ARRAY_BUFFER, textureCoords_vbo[0]);
		glEnableVertexAttribArray(textureCoordsAttribId);
		glVertexAttribPointer(textureCoordsAttribId, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

		// provide the vertex normals to the shaders
		glBindBuffer(GL_ARRAY_BUFFER, normals_vbo[0]);
		glEnableVertexAttribArray(normalAttribId);
		glVertexAttribPointer(normalAttribId, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

		// draw the triangles
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer[0]);
		glDrawElements(GL_TRIANGLES, numVertices[0], GL_UNSIGNED_INT, (void*)0);
	}

	//Draw second plane

	{
		glm::vec3 myColour = glm::vec3(1.0f, 0.0f, 0.0f);
		glm::mat4 modelMat = translationMatrixPlaneTwo;


		// Upload the entity-specific values
		glUniformMatrix4fv(transformation_mat_loc, 1, GL_FALSE,
			&modelMat[0][0]);
		glUniform3fv(colour_loc, 1, (GLfloat*)&myColour[0]);


		// find the names (ids) of each vertex attribute
  		GLint positionAttribId = glGetAttribLocation(basic_prog_ID, "position");
 		GLint textureCoordsAttribId = glGetAttribLocation(basic_prog_ID, "textureCoords");
  		GLint normalAttribId = glGetAttribLocation(basic_prog_ID, "normal");

		// provide the vertex positions to the shaders
  		glBindBuffer(GL_ARRAY_BUFFER, positions_vbo[0]);
  		glEnableVertexAttribArray(positionAttribId);
		glVertexAttribPointer(positionAttribId, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

		// provide the vertex texture coordinates to the shaders
		glBindBuffer(GL_ARRAY_BUFFER, textureCoords_vbo[0]);
		glEnableVertexAttribArray(textureCoordsAttribId);
		glVertexAttribPointer(textureCoordsAttribId, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

		// provide the vertex normals to the shaders
		glBindBuffer(GL_ARRAY_BUFFER, normals_vbo[0]);
		glEnableVertexAttribArray(normalAttribId);
		glVertexAttribPointer(normalAttribId, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

		// draw the triangles
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer[0]);
		glDrawElements(GL_TRIANGLES, numVertices[0], GL_UNSIGNED_INT, (void*)0);
	}

	glUseProgram(0);

	// TODO

	// Send the data drawn to the UI window
	glutSwapBuffers();
}

static void control(float dt, float ds) {

	// * * * Handle Camera Updates * * * //
	if (view_mode == CAMERA_FREE) {

		// Get the speed updates
		float Y_SPEED = 0.5f, XZ_SPEED = 0.5f;
		float dx = 0.0f, dy = 0.0f, dz = 0.0f;
		if (key_states[static_cast<int>('w')]) {
			dz -= XZ_SPEED;
		} if (key_states[static_cast<int>('s')]) {
			dz += XZ_SPEED;
		} if (special_keys[SP_KEY_LEFT]) {
			dx -= XZ_SPEED;
		} if (special_keys[SP_KEY_RIGHT]) {
			dx += XZ_SPEED;
		} if (key_states[32]) {
			dy += Y_SPEED;
		} if (key_states[9]) {
			dy -= Y_SPEED;
		}
		float ax = dx * cos(camera.yaw) - dz * sin(camera.yaw);
		float az = dx * sin(camera.yaw) + dz * cos(camera.yaw);

		// Camera angle changes
		float PITCH_SPEED = 0.025f, YAW_SPEED = 0.025f;
		float dp = 0.0f, dyaw = 0.0f;
		if (key_states[static_cast<int>('a')]) {
			dyaw -= YAW_SPEED;
		} if (key_states[static_cast<int>('d')]) {
			dyaw += YAW_SPEED;
		} if (special_keys[SP_KEY_UP]) {
			dp -= PITCH_SPEED;
		} if (special_keys[SP_KEY_DOWN]) {
			dp += PITCH_SPEED;
		}

		// Update the camera
		camera.x += ax;
		camera.y += dy;
		camera.z += az;
		camera.pitch += dp;
		camera.yaw += dyaw;
	}
}

float planeRotation1 = 0.0f;
float planeRotation2 = 0.0f;
float timeloop = 0.0f;

// Route in 3d space 20s trip
glm::vec3 startpoints[] = {
	//Start
	glm::vec3(250.0f, 50.0f, -125.0f),
	glm::vec3(0.0f, 50.0f, -125.0f),
	glm::vec3(0.0f, 50.0f, 0.0f),
	glm::vec3(0.0f, 0.0f, 0.0f),
	glm::vec3(0.0f, 0.0f, -150.0f)

};

glm::vec3 middlepoints1[] = {
	glm::vec3(166.0f, 50.0f, -125.0f),
	glm::vec3(0.0f, 50.0f, -75.0f),
	glm::vec3(0.0f, 30.0f, 20.0f),
	glm::vec3(0.0f, 0.0f, -50.0f),
	glm::vec3(0.0f, 0.0f, -170.0f)
};

glm::vec3 middlepoints2[] = {
	glm::vec3(82.0f, 50.0f, -125.0f),
	glm::vec3(0.0f, 50.0f, -25.0f),
	glm::vec3(0.0f, 10.0f, 20.0f),
	glm::vec3(0.0f, 0.0f, -100.0f),
	glm::vec3(0.0f, 0.0f, -190.0f)
};
glm::vec3 endpoints[] = {
	startpoints[1],
	startpoints[2],
	startpoints[3],
	startpoints[4],
	glm::vec3(0.0f, 0.0f, -200.0f)
};

int curvepos = 0;
glm::vec3 plane2path = glm::vec3(0.0f, 0.0f, 0.0f);
float planeRot = 0.0f;
bool isPlaneRot = true;
bool isPlaneRot2 = false;

static void update(void) {

    // Calculate dt
    int milliseconds = glutGet(GLUT_ELAPSED_TIME);
    delta_seconds = (milliseconds - elapsed) / 1000.0f;
    elapsed = milliseconds;
    float delta_t = delta_seconds * TIME_FACTOR;

	// Perform keyboard updates
	control(delta_t, delta_seconds);

	timeloop += delta_t;

	if (timeloop > 5.0f) {
		timeloop = 0.0f;
		curvepos += 1;
		if (curvepos == 4) {
			curvepos = 0;
		}
		if (curvepos == 1) {
			planeRot = 0.0f;
		}

	}

	if (!isPlaneRot) {
		planeRot = 0.0f;
		isPlaneRot = true;
	}

    // Find the indices of plane 1/2
    int p1 = 0, p2 = 0, idx = 0;
    for (auto e : basic_entities) {
        if (e.type == TYPE_PLANE) {
            if (e.id == PLANE1_ID) {
                p1 = idx;
            } else {
                p2 = idx;
            }
        }
        idx ++;
    }

    // Perform entity updates
	{

		//Plane 1
		glm::mat4 lastPosition = translationMatrixPlaneOne;
		translationMatrixPlaneOne = glm::mat4(1.0f);


		//Bezier interpolate along points
		glm::vec3 translationVector = glm::vec3(0.0f);
		translationVector.x = getDim(timeloop / 5.0f, startpoints[curvepos].x, middlepoints1[curvepos].x, middlepoints2[curvepos].x, endpoints[curvepos].x);
		translationVector.y = getDim(timeloop / 5.0f, startpoints[curvepos].y, middlepoints1[curvepos].y, middlepoints2[curvepos].y, endpoints[curvepos].y);
		translationVector.z = getDim(timeloop / 5.0f, startpoints[curvepos].z, middlepoints1[curvepos].z, middlepoints2[curvepos].z, endpoints[curvepos].z);

		//Translate it onto a temp matrix
		translationMatrixPlaneOne = glm::translate(translationMatrixPlaneOne, translationVector);


		// Get the direction vector
		float dx = translationMatrixPlaneOne[3].x - lastPosition[3].x;
		float dy = translationMatrixPlaneOne[3].y - lastPosition[3].y;
		float dz = translationMatrixPlaneOne[3].z - lastPosition[3].z;

		// Scale
		//translationMatrixPlaneOne = glm::scale(translationMatrixPlaneOne, glm::vec3(-0.1f, 0.1f, 0.1f));

		float d = glm::sqrt(dx * dx + dy * dy + dz * dz);

		if (d < 0) {
			d = 1.0f;
		}

		float vx = dx/d * delta_t;
		float vy = dy/d * delta_t;
		float vz = dz/d * delta_t;

		glm::vec3 velocity = glm::vec3(vx, vy, vz);
		glm::vec3 direction = glm::normalize(velocity);

		glm::mat4 rotationAndTranslation = glm::mat4(1.0f);
		glm::vec3 left = glm::vec3(0.0f);
		glm::vec3 up = glm::vec3(0.0f);


		// Calculate rotations based on direction it's travelling + keyframe specific
		switch(curvepos) {
			case 0:
				std::cout << translationMatrixPlaneOne[3].x << "\t" << planeRot << std::endl;
				if (translationMatrixPlaneOne[3].x <= 50.0f) {
					if (planeRot > 90.0f) {
						planeRot = 90.0f;
					} else {
						planeRot += delta_t * 90.0f;
					}
				}
				left = glm::cross(glm::vec3(1, 1, 0), direction);
				up = glm::cross(-direction, left);
				rotationAndTranslation[0] = glm::vec4(left, 0.0f);
				rotationAndTranslation[1] = glm::vec4(up, 0.0f);
				rotationAndTranslation = glm::rotate(rotationAndTranslation, glm::radians(planeRot), glm::vec3(0.0f, 1.0f, 1.0f));
				break;
			case 1:
				//TODO: A 180 here
				if (planeRot > 180.0f) {
					planeRot = 180.0f;
				} else {
					planeRot += delta_t * 40.0f;
				}
				std::cout << planeRot << std::endl;
				left = glm::cross(glm::vec3(0, 1, 0), direction);
				up = glm::cross(-direction, left);
				rotationAndTranslation[0] = glm::vec4(left, 0.0f);
				rotationAndTranslation[1] = glm::vec4(up, 0.0f);
				rotationAndTranslation = glm::rotate(rotationAndTranslation, glm::radians(planeRot), glm::vec3(0.0f, 0.0f, 1.0f));
				break;
			case 2:
				//Upside Down
				isPlaneRot = false;
				up = glm::cross(glm::vec3(1, 1, 0), direction);
				left = glm::cross(-direction, up);
				rotationAndTranslation[0] = glm::vec4(left, 0.0f);
				rotationAndTranslation[1] = glm::vec4(up, 0.0f);
				rotationAndTranslation = glm::rotate(rotationAndTranslation, glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
				break;
			case 3:
				left = glm::cross(glm::vec3(0, 1, 0), direction);
				up = glm::cross(-direction, left);
				rotationAndTranslation[0] = glm::vec4(left, 0.0f);
				rotationAndTranslation[1] = glm::vec4(up, 0.0f);
				break;

			case 4:
				left = glm::cross(glm::vec3(0, 1, 0), direction);
				up = glm::cross(-direction, left);
				rotationAndTranslation[0] = glm::vec4(left, 0.0f);
				rotationAndTranslation[1] = glm::vec4(up, 0.0f);
				break;
		}


		rotationAndTranslation[2] = glm::vec4(direction, 0.0f);
		rotationAndTranslation[3] = glm::vec4(translationMatrixPlaneOne[3]);

		glm::mat4 finishedMat = glm::scale(rotationAndTranslation, glm::vec3(-0.1f));

		translationMatrixPlaneOne = finishedMat;

	}

	{
		// Plane 2
		plane2path.z -= 12.5f * delta_t;

		if (plane2path.z <= -250.0f) {
			plane2path.z = 0.0f;
		}
		translationMatrixPlaneTwo = glm::mat4(1.0f);

		translationMatrixPlaneTwo = glm::translate(translationMatrixPlaneTwo, plane2path);

		// Scale
		translationMatrixPlaneTwo = glm::scale(translationMatrixPlaneTwo, glm::vec3(0.1f));

	}

    // TODO

    // Update the background colour
    glm::vec3 dark = glm::vec3(0.25098f, 0.15686f, 0.290196f);
    glm::vec3 light = glm::vec3(0.941176f, 0.494118f, 0.027451f);
    float ax = cos(camera.yaw) - sin(camera.yaw);
    float az = sin(camera.yaw) + cos(camera.yaw);
    glm::vec3 d = glm::normalize(glm::vec3(ax, 0, az));
    float i = fmax(glm::dot(d, sun_dir), 0.0f);
    background = glm::vec3(0.25098f, 0.15686f, 0.290196f);
    //background = light * i + dark * (1 - i);

    glutPostRedisplay();
}

static void spHandle(int key, bool is_down) {

	// Handle special key presses
	if (key == GLUT_KEY_UP) {
		special_keys[SP_KEY_UP] = is_down;
	} else if (key == GLUT_KEY_LEFT) {
		special_keys[SP_KEY_LEFT] = is_down;
	} else if (key == GLUT_KEY_RIGHT) {
		special_keys[SP_KEY_RIGHT] = is_down;
	} else if (key == GLUT_KEY_DOWN) {
		special_keys[SP_KEY_DOWN] = is_down;
	}
}

static void spDown(int key, int x, int y) {
	spHandle(key, true);
}

static void spUp(int key, int x, int y) {
	spHandle(key, false);
}

int main(int argc, char** argv) {

	std::cout << "===============================================" << std::endl;
	std::cout << TITLE << std::endl;
	std::cout << "===============================================" << std::endl;

	// Setup glut
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(width, height);
	window_id = glutCreateWindow(TITLE);
	glutIdleFunc(&update);
	glutDisplayFunc(&render);
	glutReshapeFunc(&reshape);
	glutMotionFunc(&drag);
	glutMouseFunc(&mouse);
	glutKeyboardFunc(&keyboard);
	glutKeyboardUpFunc(&keyboardUp);
	glutSpecialFunc(&spDown);
	glutSpecialUpFunc(&spUp);

	// Initialize glew
	glewInit();
	if (!GLEW_VERSION_2_0) {
		std::cerr << "OpenGL 2.0 not available" << std::endl;
		return 1;
	}
	std::cout << "Initializing program..." << std::endl;

	// Get version info
	const GLubyte* renderer = glGetString(GL_RENDERER);
	const GLubyte* version = glGetString(GL_VERSION);
	std::cout << "Renderer: " << renderer << std::endl;
	std::cout << "OpenGL version supported: " << version << std::endl;

	// OpenGL settings across the application
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	// Make sure none of the keys states are true
	special_keys[SP_KEY_UP] = false;
	special_keys[SP_KEY_LEFT] = false;
	special_keys[SP_KEY_RIGHT] = false;
	special_keys[SP_KEY_DOWN] = false;
	for (int i = 0; i < 256; i ++) {
		key_states[i] = false;
	}

	// Create the initial matrices
	projection_mat = getProjectionMatrix();
	view_mat = getViewMatrix(camera);

	// Create the shader programs
	basic_prog_ID = createProgram("basic.vs", "basic.fs",
		{"position", "normal"});
	if (basic_prog_ID == 0) {
		std::cerr << "Error: could not create basic shader program." << std::endl;
		return 2;
	}
	camera = Camera(-250, 25, -125);
    camera.yaw = PI / 2;
	oc.setDefaultColour(0.0f, 0.4745f, 0.75686f);
	init();

	// Run the main program
	std::cout << "Done initializing program." << std::endl;
	glutMainLoop();

	return 0;
}

/* Loads a shader into memory and returns the corresponding shader ID. If the
 * shader fails to compile or a file I/O error occurs, 0 is returned.
 * type	the type of shader file (e.g. GL_VERTEX_SHADER)
 * file	the path to the shader file to load
 *
 * Returns the ID of the shader that was compiled and loaded into memory.
 */
GLuint loadShader(const GLenum& type, const char* file) {

	FILE* fid;
	char* data;
	int len, n;

	// Open the file
	std::cout << "Loading shader file '" << file << "'..." << std::endl;
	fid = fopen(file, "r");
	if (fid == NULL) { // failed
		printf("ERROR (from loadShader(...)): Can't open shader file: '%s'\n", file);
		return 0;
	}

	// Read the data
	fseek(fid, 0, SEEK_END);
	len = ftell(fid);
	rewind(fid);
	data = new char[len + 1];
	n = fread(data, sizeof(char), len, fid);
	data[n] = 0;

	// No data
	if (data == 0) { return 0; }

	// Create the shader
	GLuint shader_ID = glCreateShader(type);
	glShaderSource(shader_ID, 1, (const  GLchar **) &data, 0);
	glCompileShader(shader_ID);

	// Check that the shader compiled properly
	int compiled;
	glGetShaderiv(shader_ID, GL_COMPILE_STATUS, &compiled);
	if (compiled != GL_TRUE) { // print any compiler errors
		char* buffer;
		printf("Shader compile error in '%s':\n", file);
		glGetShaderiv(shader_ID, GL_INFO_LOG_LENGTH, &len);
		buffer = new char[len];
		glGetShaderInfoLog(shader_ID, len, 0, buffer);
		printf("%s\n", buffer);
		delete buffer;
		return 0;
	} else {
		std::cout << "Shader file '" << file << "' compiled successfully." << std::endl;
	}

	return shader_ID;
}

/* Creates a shader program with a vertex and fragment shader.
 * vertex_file	the name of the vertex shader file
 * frag_file	the name of the fragment shader file
 * attribs		the list of attributes in the shader program
 *				(e.g. {"vertex", "normal"}) - must match the exact names in
 *				the shaders.
 *
 * Returns the ID of the shader program created or 0 if one of the shaders
 * could not be loaded.
 */
int createProgram(const char* vertex_file, const char* frag_file,
	std::list<std::string> attribs) {

	// Load the shaders
	std::string base = DIR_SHADERS;
	std::string f1 = base + vertex_file;
	std::string f2 = base + frag_file;
	int vs = loadShader(GL_VERTEX_SHADER, f1.c_str());
	if (vs <= 0) {
		return 0;
	}
	int fs = loadShader(GL_FRAGMENT_SHADER, f2.c_str());
	if (fs <= 0) {
		return 0;
	}

	// Create the program
	int pid = glCreateProgram();
	glAttachShader(pid, vs);
	glAttachShader(pid, fs);

	// Bind the attribute locations
	int idx = 0;
	for (auto attrib : attribs) {
		glBindAttribLocation(pid, idx++, attrib.c_str());
	}

	// Finish building the program
	glLinkProgram(pid);
	glValidateProgram(pid);

	return pid;
}

/* Initializes the program by creating all the geometry. */
void init() {

	// Create the terrain
	VBOList vbos = createTerrain(oc, 500.0f, 500.0f, 5.0f, -5.0f);
	Entity terrain;
	terrain.vbos = vbos;
	terrain.y = -100.0f;
	terrain.colour = glm::vec3(0.5f, 1.0f, 0.5f);
	basic_entities.push_back(terrain);

    // Load the plane
    VBOList plane = loadObject(oc, DIR_MESHES"Mig_29.obj");
    numVertices[0] = plane.vertices;
    positions_vbo[0] = plane.positions_vbo;
    textureCoords_vbo[0] = plane.uv_vbo;
    normals_vbo[0] = plane.normals_vbo;
    indexBuffer[0] = plane.indices_vbo;

    // Create the planes
    Entity plane1;
    plane1.vbos = plane;
    plane1.colour = glm::vec3(0.0f, 0.0f, 1.0f);
    plane1.type = TYPE_PLANE;
    plane1.id = PLANE1_ID;
    Entity plane2;
    plane2.vbos = plane;
    plane2.colour = glm::vec3(1.0f, 0.0f, 0.0f);
    plane2.type = TYPE_PLANE;
    plane2.id = PLANE1_ID + 1;

  /*
    //Plane 2
    mesh.load("models/10593_Fighter_Jet_SG_v1_iterations-2.obj", false, false);

    numVertices[1] = mesh.getNumIndexedVertices();
    vertexPositions = mesh.getIndexedPositions();
    vertexTextureCoords = mesh.getIndexedTextureCoords();
    vertexNormals = mesh.getIndexedNormals();

    glGenBuffers(1, &positions_vbo[1]);
    glBindBuffer(GL_ARRAY_BUFFER, positions_vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, numVertices[1] * sizeof(glm::vec3), vertexPositions, GL_STATIC_DRAW);

    glGenBuffers(1, &textureCoords_vbo[1]);
    glBindBuffer(GL_ARRAY_BUFFER, textureCoords_vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, numVertices[1] * sizeof(glm::vec3), vertexTextureCoords, GL_STATIC_DRAW);

    glGenBuffers(1, &normals_vbo[1]);
    glBindBuffer(GL_ARRAY_BUFFER, normals_vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, numVertices[1] * sizeof(glm::vec3), vertexNormals, GL_STATIC_DRAW);

    indexData = mesh.getTriangleIndices();
    numTriangles = mesh.getNumTriangles();

    glGenBuffers(1, &indexBuffer[1]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * numTriangles * 3, indexData, GL_STATIC_DRAW);
    */

}
