#ifndef VOXELS_VOXEL_H_
#define VOXELS_VOXEL_H_

#include "../typedefs.h"

constexpr int BLOCK_DIR_NORTH = 0x0;
constexpr int BLOCK_DIR_WEST = 0x1;
constexpr int BLOCK_DIR_SOUTH = 0x2;
constexpr int BLOCK_DIR_EAST = 0x3;
constexpr int BLOCK_DIR_UP = 0x4;
constexpr int BLOCK_DIR_DOWN = 0x5;

// limited to 16 block orientations
constexpr int BLOCK_ROT_MASK = 0xF;
// limited to 16 block variants
constexpr int BLOCK_VARIANT_MASK = 0xF0;

struct voxel {
	blockid_t id;
	uint8_t states;

	inline uint8_t rotation() const {
		return states & BLOCK_ROT_MASK;
	}

	inline int8_t variant() const {
		return (states & BLOCK_VARIANT_MASK) >> 4;
	}
};

#endif /* VOXELS_VOXEL_H_ */
