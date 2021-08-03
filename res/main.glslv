#version 330 core

layout (location = 0) in vec3 v_position;
layout (location = 1) in vec2 v_texCoord;
layout (location = 2) in vec4 v_light;

out vec4 a_color;
out vec2 a_texCoord;

uniform mat4 u_model;
uniform mat4 u_projview;
uniform vec3 u_skyLightColor;
uniform float u_gamma;

void main(){
	vec4 position = u_projview * u_model * vec4(v_position, 1.0);
	a_color = vec4(pow(v_light.rgb, vec3(u_gamma)),1.0f);
	a_texCoord = v_texCoord;
	a_color.rgb += u_skyLightColor * v_light.a*0.5;
	a_color.rgb *= 1.0-position.z*0.0025;
	gl_Position = position;
}
