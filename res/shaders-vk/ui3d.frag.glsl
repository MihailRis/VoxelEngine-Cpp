#version 450

layout(location = 0) in vec2 a_textureCoord;
layout(location = 1) in vec4 a_color;

layout(location = 0) out vec4 f_color;

layout(set = 1, binding = 0) uniform sampler2D u_texture;

void main(){
	f_color = a_color * texture(u_texture, a_textureCoord);
}
