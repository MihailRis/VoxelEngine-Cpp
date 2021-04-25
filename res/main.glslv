#version 330 core

layout (location = 0) in vec3 v_position;
layout (location = 1) in vec2 v_texCoord;
layout (location = 2) in vec4 v_light;

out vec4 a_color;
out vec2 a_texCoord;

uniform mat4 model;
uniform mat4 projview;

void main(){
	vec4 position = projview * model * vec4(v_position, 1.0);
	a_color = vec4(v_light.r,v_light.g,v_light.b,1.0f);
	a_texCoord = v_texCoord;
	a_color.rgb += v_light.a;
	a_color.rgb *= 1.0-position.z*0.0025;
	//a_color.rgb = pow(a_color.rgb, vec3(1.0/0.7));
	gl_Position = position;
}
