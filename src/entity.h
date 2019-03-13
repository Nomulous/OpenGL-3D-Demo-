#ifndef __entity_h__
#define __entity_h__

#ifndef GLM_ENABLE_EXPERIMENTAL
#define GLM_ENABLE_EXPERIMENTAL
#endif
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

inline float DEG2RAD(float deg) { return (3.141592653589793238f * deg / 180.0f); }

class VBOList {
public:
	int positions_vbo = 0;
	int normals_vbo = 0;
	int uv_vbo = 0;
	int indices_vbo = 0;
	int vertices = 0;
};

class Entity {
	public:
		Entity() {}

		/* Constructs an entity with an initial position. */
		Entity(float x, float y, float z) {
			this->x = x;
			this->y = y;
			this->z = z;
		}

		/* Constructs an entity with various transformations. */
		Entity(glm::vec3 pos, glm::vec3 scale, glm::vec3 rotation) {
			this->x = pos.x;
			this->y = pos.y;
			this->z = pos.z;
			this->sx = scale.x;
			this->sy = scale.y;
			this->sz = scale.z;
			this->rx = rotation.x;
			this->ry = rotation.y;
			this->rz = rotation.z;
		}

		/* Gets a vector with the current (x,y,z) position of the entity. */
		inline glm::vec3 GetPosition() { return glm::vec3(x, y, z); }

		/* Sets the current position of the entity. */
		inline void SetPosition(glm::vec3 pos) { SetPosition(pos.x, pos.y, pos.z); }

		/* Sets the current position of the entity. */
		inline void SetPosition(const float& x, const float& y, const float& z) {
			this->x = x;
			this->y = y;
			this->z = z;
		}

		inline void ChangePosition(const float& dx, const float& dy, const float& dz) {
			this->x += dx;
			this->y += dy;
			this->z += dz;
		}

		inline void SetRotationDegrees(const float& rx, const float& ry, const float& rz) {
			this->rx = DEG2RAD(rx);
			this->ry = DEG2RAD(ry);
			this->rz = DEG2RAD(rz);
		}
		inline void SetRotationRadians(const float& rx, const float& ry, const float& rz) {
			this->rx = rx;
			this->ry = ry;
			this->rz = rz;
		}
		inline void ChangeRotationRadians(const float& rx, const float& ry, const float& rz) {
			this->rx += rx;
			this->ry += ry;
			this->rz += rz;
		}

		/* Gets a vector with the current x, y, and z scaling. */
		inline glm::vec3 GetScale() { return glm::vec3(sx, sy, sz); }

		/* Sets the (x,y,z) scale for the entity. */
		inline void SetScale(const float& sx, const float& sy, const float& sz) {
			this->sx = sx;
			this->sy = sy;
			this->sz = sz;
		}

		/* Gets the transformation matrix using the position, scale, and rotation. */
		inline glm::mat4 GetTransformationMatrix() const {
			glm::mat4 identity = glm::mat4(1.0f);
			glm::mat4 translation = glm::translate(identity, glm::vec3(x, y, z));
			glm::mat4 scaling = glm::scale(identity, glm::vec3(sx, sy, sz));
			glm::mat4 rotation = glm::rotate(identity, rx, glm::vec3(1, 0, 0));
			rotation = glm::rotate(rotation, ry, glm::vec3(0, 1, 0));
			rotation = glm::rotate(rotation, rz, glm::vec3(0, 0, 1));
			return translation * rotation * scaling;
		}

		glm::vec3 colour;

		float x = 0, y = 0, z = 0;		// position of entity

		int tid = 0;
		VBOList vbos;

		int type = 0, id = 0;

		bool visible = true;
	private:
		float rx = 0, ry = 0, rz = 0;	// rotation values: x, y, z (radians)
		float sx = 1, sy = 1, sz = 1;	// scale factor: x, y, z
};

class Camera {
	public:
		Camera() {}
		Camera(float x, float y, float z) : x(x), y(y), z(z) { }
		float x = 0, y = 0, z = 0;
		float pitch = 0;
		float yaw = 0;
		float roll = 0;

		inline glm::vec3 getPosition() {
			return glm::vec3(x, y, z);
		}
};

#endif
