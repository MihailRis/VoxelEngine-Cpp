#ifndef SRC_CONSTANTS_H_
#define SRC_CONSTANTS_H_

#include "typedefs.h"

#define CHUNK_W 16
#define CHUNK_H 256
#define CHUNK_D 16
#define CHUNK_VOL (CHUNK_W * CHUNK_H * CHUNK_D)

#define BLOCK_VOID (blockid_t)((2 << (sizeof(blockid_t)*8)) - 1)

inline uint vox_index(int x, int y, int z, int w, int d) {
	return (y * d + z) * w + x;
}

#endif // SRC_CONSTANTS_H_