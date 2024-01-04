#ifndef VOXELS_VOXEL_H_
#define VOXELS_VOXEL_H_

#include "../typedefs.h"

const int BLOCK_DIR_NORTH = 0x0;
const int BLOCK_DIR_WEST = 0x1;
const int BLOCK_DIR_SOUTH = 0x2;
const int BLOCK_DIR_EAST = 0x3;
const int BLOCK_DIR_UP = 0x4;
const int BLOCK_DIR_DOWN = 0x5;

// limited to 8 block orientations
const int BLOCK_ROT_MASK =     0b0000'0111;
// limited to 32 block variants
const int BLOCK_VARIANT_MASK = 0b1111'1000;

struct voxel {
	blockid_t id;
	blockstate_t states;

	inline uint8_t rotation() const {
		return states & BLOCK_ROT_MASK;
	}

	inline uint8_t variant() const {
		return (states & BLOCK_VARIANT_MASK) >> 3;
	}
};

#endif /* VOXELS_VOXEL_H_ */
