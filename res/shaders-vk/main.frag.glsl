#version 450 core

layout(location = 0) in vec4 a_color;
layout(location = 1) in vec2 a_texCoord;
layout(location = 2) in float a_distance;
layout(location = 3) in vec3 a_dir;

layout(location = 0) out vec4 f_color;

layout(set = 1, binding = 0) uniform sampler2D u_texture0;
layout(set = 1, binding = 1) uniform samplerCube u_cubemap;

layout (set = 0, binding = 1) uniform Fog {
    float u_fogFactor;
    float u_fogCurve;
};

void main(){
    vec3 dir = a_dir;
    dir.z = -dir.z;
    vec3 fogColor = texture(u_cubemap, dir).rgb;
    vec4 tex_color = texture(u_texture0, a_texCoord);
    float depth = (a_distance/256.0);
    float alpha = a_color.a * tex_color.a;
    // anyway it's any alpha-test alternative required
    if (alpha < 0.1f)
    discard;
    f_color = mix(a_color * tex_color, vec4(fogColor,1.0), min(1.0, pow(depth*u_fogFactor, u_fogCurve)));
    f_color.a = alpha;
}
