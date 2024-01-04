#version 450

#include "../shaders/lib/commons.glsl"

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec2 v_texCoord;
layout(location = 2) in float v_light;

layout(location = 0) out vec4 a_color;
layout(location = 1) out vec2 a_texCoord;
layout(location = 2) out float a_distance;
layout(location = 3) out vec3 a_dir;

layout(set = 0, binding = 0) uniform State {
    mat4 u_proj;
    mat4 u_view;
    vec3 u_cameraPos;
    float u_gamma;
};

layout(push_constant) uniform Dynamic {
    mat4 u_model;
    vec3 u_torchlightColor;
    float u_torchlightDistance;
};

layout(set = 1, binding = 1) uniform samplerCube u_cubemap;

#define SKY_LIGHT_MUL 2.5

void main(){
    vec3 pos3d = (u_model * vec4(v_position, 1.0)).xyz-u_cameraPos.xyz;
    vec4 modelpos = u_model * vec4(v_position, 1.0);
    vec4 viewmodelpos = u_view * modelpos;
    vec4 decomp_light = decompress_light(v_light);
    vec3 light = decomp_light.rgb;
    float torchlight = max(0.0, 1.0-distance(u_cameraPos, modelpos.xyz)/u_torchlightDistance);
    a_dir = modelpos.xyz - u_cameraPos;
    light += torchlight * u_torchlightColor;
    a_color = vec4(pow(light, vec3(u_gamma)),1.0f);
    a_texCoord = v_texCoord;

    vec3 skyLightColor = texture(u_cubemap, vec3(-0.4f, 0.05f, -0.4f)).rgb;
    skyLightColor.g *= 0.9;
    skyLightColor.b *= 0.8;
    skyLightColor = min(vec3(1.0), skyLightColor*SKY_LIGHT_MUL);

    a_color.rgb = max(a_color.rgb, skyLightColor.rgb*decomp_light.a);
    a_distance = length(u_view * u_model * vec4(pos3d.x, pos3d.y*0.2, pos3d.z, 0.0));
    gl_Position = u_proj * viewmodelpos;
}
