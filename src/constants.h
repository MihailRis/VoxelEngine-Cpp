#ifndef SRC_CONSTANTS_H_
#define SRC_CONSTANTS_H_

#include <climits>
#include "typedefs.h"

#define ENGINE_VERSION_MAJOR 0
#define ENGINE_VERSION_MINOR 15
#define STR_(x) #x
#define STR(x) STR_(x)
#define ENGINE_VERSION STR(ENGINE_VERSION_MAJOR) "." STR(ENGINE_VERSION_MINOR)
#define MAKE_VERSION(major, minor, patch) \
			((((uint32_t)(major)) << 22U) | (((uint32_t)(minor)) << 12U) | ((uint32_t)(patch)))
#define ENGINE_VERSION_VALUE MAKE_VERSION(0, 15, 0)


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

#ifdef USE_VULKAN
constexpr bool VULKAN_USES = true;
#else
constexpr bool VULKAN_USES = false;
#endif

#endif // SRC_CONSTANTS_H_
