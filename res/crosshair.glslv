#version 330 core

layout (location = 0) in vec2 v_position;

uniform float u_ar;
uniform float u_scale;

void main(){
	gl_Position = vec4(v_position.x * u_ar * u_scale, v_position.y * u_scale, 0.0, 1.0);
}
