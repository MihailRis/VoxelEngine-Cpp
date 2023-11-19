#version 330 core

in vec2 a_textureCoord;
in vec4 a_color;
out vec4 f_color;

uniform sampler2D u_texture;

void main(){
	f_color = a_color * texture(u_texture, a_textureCoord);
}
