// Example of a GLSL library

vec4 decompress_light(float compressed_light) {
	vec4 result;
    int compressed = floatBitsToInt(compressed_light);
    result.r = ((compressed >> 24) & 0xFF) / 255.f;
    result.g = ((compressed >> 16) & 0xFF) / 255.f;
    result.b = ((compressed >> 8) & 0xFF) / 255.f;
    result.a = (compressed & 0xFF) / 255.f;
	return result;
}
