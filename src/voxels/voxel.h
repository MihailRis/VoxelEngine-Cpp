#ifndef VOXELS_VOXEL_H_
#define VOXELS_VOXEL_H_

#include "../typedefs.h"

#define BLOCK_DIR_X 0x1
#define BLOCK_DIR_Y 0x0
#define BLOCK_DIR_Z 0x2

// limited to 16 block orientations
const int BLOCK_ROT_MASK = 0xF;

struct voxel {
	blockid_t id;
	uint8_t states;
};

#endif /* VOXELS_VOXEL_H_ */
