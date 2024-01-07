#version 450

layout (location = 0) in vec2 v_position;

layout(location = 0) out vec3 v_coord;

layout(set = 0, binding = 0) uniform Background {
	mat4 u_view;
	float u_ar;
	float u_zoom;
};

// TODO: delete v_position
void main(){
	v_coord = (vec4(v_position * vec2(u_ar, 1.0f) * u_zoom, -1.0, 1.0) * u_view).xyz;
	gl_Position = vec4(v_position, 0.0, 1.0);
}
