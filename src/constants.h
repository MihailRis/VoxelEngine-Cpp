#ifndef SRC_CONSTANTS_H_
#define SRC_CONSTANTS_H_

#include <climits>
#include <limits>

#include "typedefs.h"

constexpr int ENGINE_VERSION_MAJOR = 0;
constexpr int ENGINE_VERSION_MINOR = 17;
#define MAKE_VERSION(major, minor, patch) \
			((((uint32_t)(major)) << 22U) | (((uint32_t)(minor)) << 12U) | ((uint32_t)(patch)))
constexpr uint32_t ENGINE_VERSION_VALUE = MAKE_VERSION(0, 15, 0);

constexpr int CHUNK_W = 16;
constexpr int CHUNK_H = 256;
constexpr int CHUNK_D = 16;

const uint VOXEL_USER_BITS = 8;
constexpr uint VOXEL_USER_BITS_OFFSET = sizeof(blockstate_t)*8-VOXEL_USER_BITS;

/* Chunk volume (count of voxels per Chunk) */
constexpr int CHUNK_VOL = (CHUNK_W * CHUNK_H * CHUNK_D);

/* BLOCK_VOID is block id used to mark non-existing voxel (voxel of missing chunk) */
constexpr blockid_t BLOCK_VOID = std::numeric_limits<blockid_t>::max();
constexpr blockid_t MAX_BLOCKS = BLOCK_VOID;

inline uint vox_index(int x, int y, int z, int w=CHUNK_W, int d=CHUNK_D) {
	return (y * d + z) * w + x;
}

//cannot replace defines with const while used for substitution
#define SHADERS_FOLDER "shaders"
#define TEXTURES_FOLDER "textures"
#define FONTS_FOLDER "fonts"

#ifdef USE_VULKAN
constexpr bool VULKAN_USES = true;
#else
constexpr bool VULKAN_USES = false;
#endif

#endif // SRC_CONSTANTS_H_
