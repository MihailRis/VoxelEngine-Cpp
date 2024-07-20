#ifndef COMMONS_GLSL_
#define COMMONS_GLSL_

#include <constants>

#define PI 3.1415926535897932384626433832795
#define PI2 (PI*2)

vec4 decompress_light(float compressed_light) {
    vec4 result;
    int compressed = floatBitsToInt(compressed_light);
    result.r = ((compressed >> 24) & 0xFF) / 255.f;
    result.g = ((compressed >> 16) & 0xFF) / 255.f;
    result.b = ((compressed >> 8) & 0xFF) / 255.f;
    result.a = (compressed & 0xFF) / 255.f;
    return result;
}

vec3 pick_sky_color(samplerCube cubemap) {
    vec3 skyLightColor = texture(cubemap, vec3(0.4f, 0.0f, 0.4f)).rgb;
    skyLightColor *= SKY_LIGHT_TINT;
    skyLightColor = min(vec3(1.0), skyLightColor*SKY_LIGHT_MUL);
    skyLightColor = max(MAX_SKY_LIGHT, skyLightColor);
    return skyLightColor;
}

#endif // COMMONS_GLSL_
