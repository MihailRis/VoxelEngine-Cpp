#ifndef SRC_CONSTANTS_H_
#define SRC_CONSTANTS_H_

#include <limits.h>
#include "typedefs.h"

#define CHUNK_W 16
#define CHUNK_H 256
#define CHUNK_D 16

/* Chunk volume (count of voxels per Chunk) */
#define CHUNK_VOL (CHUNK_W * CHUNK_H * CHUNK_D)

/* BLOCK_VOID is block id used to mark non-existing voxel (voxel of missing chunk) */
#define BLOCK_VOID (blockid_t)((2 << (sizeof(blockid_t)*CHAR_BIT)) - 1)

inline uint vox_index(int x, int y, int z, int w=CHUNK_W, int d=CHUNK_D) {
	return (y * d + z) * w + x;
}

#define SHADERS_FOLDER "shaders"
#define TEXTURES_FOLDER "textures"
#define FONTS_FOLDER "fonts"

#endif // SRC_CONSTANTS_H_
