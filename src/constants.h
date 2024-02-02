#ifndef SRC_CONSTANTS_H_
#define SRC_CONSTANTS_H_

#include <limits>
#include "typedefs.h"

const int ENGINE_VERSION_MAJOR = 0;
const int ENGINE_VERSION_MINOR = 19;

const int BLOCK_AIR = 0;
const int ITEM_EMPTY = 0;

const int CHUNK_W = 16;
const int CHUNK_H = 256;
const int CHUNK_D = 16;

const uint VOXEL_USER_BITS = 8;
constexpr uint VOXEL_USER_BITS_OFFSET = sizeof(blockstate_t)*8-VOXEL_USER_BITS;

const int ITEM_ICON_SIZE = 48;

/* Chunk volume (count of voxels per Chunk) */
constexpr int CHUNK_VOL = (CHUNK_W * CHUNK_H * CHUNK_D);

/* BLOCK_VOID is block id used to mark non-existing voxel (voxel of missing chunk) */
const blockid_t BLOCK_VOID = std::numeric_limits<blockid_t>::max();
const itemid_t ITEM_VOID = std::numeric_limits<itemid_t>::max();

const blockid_t MAX_BLOCKS = BLOCK_VOID;

constexpr uint vox_index(uint x, uint y, uint z, uint w=CHUNK_W, uint d=CHUNK_D) {
	return (y * d + z) * w + x;
}

//cannot replace defines with const while used for substitution
#define SHADERS_FOLDER "shaders" 
#define TEXTURES_FOLDER "textures"
#define FONTS_FOLDER "fonts"

#endif // SRC_CONSTANTS_H_
