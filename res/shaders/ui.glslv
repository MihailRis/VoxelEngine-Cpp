#version 330 core

layout (location = 0) in vec2 v_position;
layout (location = 1) in vec2 v_textureCoord;
layout (location = 2) in vec4 v_color;

out vec2 a_textureCoord;
out vec4 a_color;

uniform mat4 u_projview;

void main(){
	a_textureCoord = v_textureCoord;
	a_color = v_color;
	gl_Position = u_projview * vec4(v_position, 0.5, 1.0);
}
