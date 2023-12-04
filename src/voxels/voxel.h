#ifndef VOXELS_VOXEL_H_
#define VOXELS_VOXEL_H_

#include "../typedefs.h"

#define BLOCK_DIR_PX 0x1
#define BLOCK_DIR_PY 0x0
#define BLOCK_DIR_PZ 0x2
#define BLOCK_DIR_MX 0x3
#define BLOCK_DIR_MY 0x4
#define BLOCK_DIR_MZ 0x5

// limited to 16 block orientations
#define BLOCK_ROT_MASK 0xF

struct voxel {
	blockid_t id;
	uint8_t states;
};

#endif /* VOXELS_VOXEL_H_ */
