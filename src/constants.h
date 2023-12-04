#ifndef SRC_CONSTANTS_H_
#define SRC_CONSTANTS_H_

#include <limits.h>
#include "typedefs.h"

const int ENGINE_VERSION_MAJOR = 0;
const int ENGINE_VERSION_MINOR = 15;

const int CHUNK_W = 16;
const int CHUNK_H = 256;
const int CHUNK_D = 16;

/* Chunk volume (count of voxels per Chunk) */
const int CHUNK_VOL = (CHUNK_W * CHUNK_H * CHUNK_D);

/* BLOCK_VOID is block id used to mark non-existing voxel (voxel of missing chunk) */
const blockid_t BLOCK_VOID = UCHAR_MAX;

inline uint vox_index(int x, int y, int z, int w=CHUNK_W, int d=CHUNK_D) {
	return (y * d + z) * w + x;
}

//cannot replace defines with const while used for substitution
#define SHADERS_FOLDER "shaders" 
#define TEXTURES_FOLDER "textures"
#define FONTS_FOLDER "fonts"

#endif // SRC_CONSTANTS_H_
