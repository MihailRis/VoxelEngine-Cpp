#version 450

layout(location = 0) in vec3 v_coord;
layout(location = 0) out vec4 f_color;

layout(set = 1, binding = 0) uniform samplerCube u_cubemap;

void main(){
	vec3 dir = normalize(v_coord);
	dir.z = -dir.z;
	f_color = texture(u_cubemap, dir);
}
