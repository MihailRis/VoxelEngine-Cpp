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

inline long max(long a, long b) {
	return (a > b) ? a : b;
}

inline long min(long a, long b) {
	return (a < b) ? a : b;
}

static u_int g_seed;

inline void fast_srand(int seed) {
	g_seed = seed;
}

inline int fast_rand(void) {
	g_seed = (214013 * g_seed + 2531011);
	return (g_seed >> 16) & 0x7FFF;
}

inline light_t light_pack(u_char r, u_char g, u_char b, u_char s) {
	return r | (g << 4) | (b << 8) | (s << 12);
}

#endif // SRC_VOXNATHS_H_
