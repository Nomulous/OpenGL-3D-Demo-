attribute vec3 position;
attribute vec3 normal;

uniform mat4 transformation_mat;
uniform mat4 projection_mat;
uniform mat4 view_mat;
uniform vec3 camera;

varying vec3 surface_normal;
varying vec4 world_pos;
varying vec3 to_camera;

void main(void) {

	world_pos = transformation_mat * vec4(position, 1.0);
	gl_Position = projection_mat * view_mat * world_pos;

	surface_normal = (transformation_mat * vec4(normal, 0.0)).xyz;
	to_camera = camera - world_pos.xyz;
}
