#ifndef VOXELS_VOXEL_H_
#define VOXELS_VOXEL_H_

#include "../typedefs.h"

const int BLOCK_DIR_NORTH = 0x0;
const int BLOCK_DIR_WEST = 0x1;
const int BLOCK_DIR_SOUTH = 0x2;
const int BLOCK_DIR_EAST = 0x3;
const int BLOCK_DIR_UP = 0x4;
const int BLOCK_DIR_DOWN = 0x5;

// limited to 16 block orientations
const int BLOCK_ROT_MASK = 0xF;
// limited to 16 block variants
const int BLOCK_VARIANT_MASK = 0xF0;

struct voxel {
	blockid_t id;
	u_short16 states;

	inline u_char8 getDir() const {
		return states & BLOCK_ROT_MASK;
	}

	inline void setDir(u_char8 dir) {
		states &= ~BLOCK_ROT_MASK;
        states |= dir;
    }

	inline u_char8 variant() const {
		return (states & BLOCK_VARIANT_MASK) >> 4;
	}
};

#endif /* VOXELS_VOXEL_H_ */
