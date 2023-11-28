#version 450

layout(location = 0) in vec2 v_position;

layout(location = 0) out vec2 v_coord;

void main(){
	v_coord = v_position;
	gl_Position = vec4(v_position, 0.0, 1.0);
}
