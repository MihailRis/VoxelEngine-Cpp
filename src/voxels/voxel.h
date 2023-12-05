#ifndef VOXELS_VOXEL_H_
#define VOXELS_VOXEL_H_

#include "../typedefs.h"

const int BLOCK_DIR_X = 0x1;
const int BLOCK_DIR_Y = 0x0;
const int BLOCK_DIR_Z = 0x2;

const int BLOCK_DIR_NORTH = 0x0;
const int BLOCK_DIR_WEST = 0x1;
const int BLOCK_DIR_SOUTH = 0x2;
const int BLOCK_DIR_EAST = 0x3;

// limited to 16 block orientations
const int BLOCK_ROT_MASK = 0xF;

struct voxel {
	blockid_t id;
	uint8_t states;
};

#endif /* VOXELS_VOXEL_H_ */
