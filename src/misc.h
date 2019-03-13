#ifndef __misc_h__
#define __misc_h__

#include <GL/glew.h>

class ObjectCreator {
public:

	// Deletes the graphics objects loaded from memory
	void destroy() {

		// Delete VAOs
		while (!vaos.empty()) {
			glDeleteVertexArrays(1, &vaos.front());
			vaos.pop_front();
		}

		// Delete VBOs
		while (!vbos.empty()) {
			glDeleteBuffers(1, &vbos.front());
			vbos.pop_front();
		}

		// Delete textures
		while (!textures.empty()) {
			glDeleteTextures(1, &textures.front());
			textures.pop_front();
		}
	}

	// Stores data in a VBO
	GLuint storeInVBO(int attr_number, int size, GLfloat* data, int length) {
		GLuint vbo_ID;
		glGenBuffers(1, &vbo_ID);
		vbos.push_front(vbo_ID);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_ID);
		glBufferData(GL_ARRAY_BUFFER, length*sizeof(GLfloat), data, GL_STATIC_DRAW);
		glVertexAttribPointer(attr_number, size, GL_FLOAT, GL_FALSE, 0, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		return vbo_ID;
	}

	// Stores the indices in a VBO
	GLuint bindIndicesBuffer(GLuint* indices, int length) {
		GLuint vbo_ID;
		glGenBuffers(1, &vbo_ID);
		vbos.push_front(vbo_ID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_ID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, length*sizeof(GLuint), indices, GL_STATIC_DRAW);

		return vbo_ID;
	}

	// Loads a colour as a texture and returns the texture ID
	GLuint loadColourTexture(const float& r, const float& g, const float& b) {

		// Create one OpenGL texture
		GLuint texture_ID;
		glGenTextures(1, &texture_ID);
		glBindTexture(GL_TEXTURE_2D, texture_ID);

		// Bind the data to the texture
		GLfloat data[3] = { r, g, b };
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_FLOAT, &data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		// Keep track of the texture ID
		textures.push_back(texture_ID);

		return texture_ID;
	}

	GLuint loadBMP(const char* image_path) {

		// Header info
		unsigned char header[54];
		unsigned int data_pos;		// where the header of BMP ends
		unsigned int width, height;
		unsigned int image_size;	// the sizeof data

		// Pixel data
		unsigned char* data;

		// Open the file
		FILE * file = fopen(image_path, "rb");
		if (!file) {
			printf("ERROR (from loadBMP(...)): Image could not be opened '%s'\n",
				image_path);
			return default_texture_ID;
		}

		// Read the header of the file
		if (fread(header, 1, 54, file) != 54) { // if 54 bytes doesn't exist, not a BMP
			printf("ERROR (from loadBMP(...)): Not a correct BMP file '%s'\n",
				image_path);
			return default_texture_ID;
		}

		// Check the file starts with BM
		if (header[0] != 'B' || header[1] != 'M') {
			printf("ERROR (from loadBMP(...)): Not a correct BMP file '%s'\n",
				image_path);
			return default_texture_ID;
		}

		// Get info from the header
		data_pos = *(int*)&(header[0x0A]);
		image_size = *(int*)&(header[0x22]);
		width = *(int*)&(header[0x12]);
		height = *(int*)&(header[0x16]);

		// Guess missing info
		if (image_size == 0) {
			image_size = width * height * 3; // 3 bytes for RGB (assuming no alpha)
		}
		if (data_pos == 0) {
			data_pos = 54;
		}

		// Read the file data into a buffer
		data = new unsigned char[image_size];
		fread(data, 1, image_size, file);
		fclose(file);

		// Create one OpenGL texture
		GLuint texture_ID;
		glGenTextures(1, &texture_ID);
		glBindTexture(GL_TEXTURE_2D, texture_ID);

		// Bind the data to the texture
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		// Keep track of the texture ID
		textures.push_back(texture_ID);

		return texture_ID;
	}

	void setDefaultColour(const float& r, const float& g, const float& b) {
		this->default_texture_ID = loadColourTexture(r, g, b);
	}

	ObjectCreator() {}

private:
	std::list<GLuint> vaos = {};
	std::list<GLuint> vbos = {};
	std::list<GLuint> textures = {};

	/* The default texture ID for this loader. */
	GLuint default_texture_ID = 0;
};

#endif
