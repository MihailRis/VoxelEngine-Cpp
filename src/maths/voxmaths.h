#ifndef SRC_VOXNATHS_H_
#define SRC_VOXNATHS_H_

#include "../typedefs.h"

inline int floordiv(int a, int b) {
	if (a < 0 && a % b) {
		return (a / b) - 1;
	}
	return a / b;
}

inline int ceildiv(int a, int b) {
	if (a > 0 && a % b) {
		return a / b + 1;
	}
	return a / b;
}

inline int max(int a, int b) {
	return (a > b) ? a : b;
}

inline int min(int a, int b) {
	return (a < b) ? a : b;
}

inline int64_t max(int64_t a, int64_t b) {
	return (a > b) ? a : b;
}

inline int64_t min(int64_t a, int64_t b) {
	return (a < b) ? a : b;
}

static unsigned int g_seed;

inline void fast_srand(int seed) {
	g_seed = seed;
}

inline int fast_rand(void) {
	g_seed = (214013 * g_seed + 2531011);
	return (g_seed >> 16) & 0x7FFF;
}

inline light_t light_pack(ubyte r, ubyte g, ubyte b, ubyte s) {
	return r | (g << 4) | (b << 8) | (s << 12);
}

#endif // SRC_VOXNATHS_H_
