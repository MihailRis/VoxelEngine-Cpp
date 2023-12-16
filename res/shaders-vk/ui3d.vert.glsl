#version 450

layout (location = 0) in vec3 v_position;
layout (location = 1) in vec2 v_textureCoord;
layout (location = 2) in vec4 v_color;

layout(location = 0) out vec2 a_textureCoord;
layout(location = 1) out vec4 a_color;

layout(set = 0, binding = 0) uniform ProjView {
	mat4 u_projview;
};

layout(set = 0, binding = 1) uniform Apply {
	mat4 u_apply;
};

void main(){
	a_textureCoord = v_textureCoord;
	a_color = v_color;
	gl_Position = u_apply * u_projview * vec4(v_position, 1.0);
}
