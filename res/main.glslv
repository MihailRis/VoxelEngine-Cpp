#version 330 core

layout (location = 0) in vec3 v_position;
layout (location = 1) in vec2 v_texCoord;
layout (location = 2) in vec4 v_light;

out vec4 a_color;
out vec2 a_texCoord;
out float a_distance;

uniform mat4 u_model;
uniform mat4 u_proj;
uniform mat4 u_view;
uniform vec3 u_skyLightColor;
uniform vec3 u_cameraPos;
uniform float u_gamma;

uniform vec3 u_torchlightColor;
uniform float u_torchlightDistance;

void main(){
	vec2 pos2d = (u_model * vec4(v_position, 1.0)).xz-u_cameraPos.xz;
	vec4 modelpos = u_model * vec4(v_position+vec3(0,pow(length(pos2d)*0.0, 3.0),0), 1.0);
	vec4 viewmodelpos = u_view * modelpos;
	vec3 light = v_light.rgb;
	float torchlight = max(0.0, 1.0-distance(u_cameraPos, modelpos.xyz)/u_torchlightDistance);
	light += torchlight * u_torchlightColor;
	a_color = vec4(pow(light, vec3(u_gamma)),1.0f);
	a_texCoord = v_texCoord;
	a_color.rgb += u_skyLightColor * v_light.a;
	a_distance = pow(length(viewmodelpos), 1.5);
	gl_Position = u_proj * viewmodelpos;
}
