varying vec3 surface_normal;
varying vec4 world_pos;
varying vec3 to_camera;

uniform vec3 light_dir;
uniform vec3 colour;

void main(void) {

	vec3 unit_normal = normalize(surface_normal);
	vec3 unit_camera = normalize(to_camera);

	// Calculate diffuse
	vec3 unit_light = -normalize(light_dir);
    float n_dot_l = dot(unit_normal, unit_light);
    float brightness = max(n_dot_l, 0.3);

	gl_FragColor = vec4(colour * brightness, 1.0);
}
