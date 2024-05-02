#ifndef SRC_CONSTANTS_H_
#define SRC_CONSTANTS_H_

#include <limits>
#include <string>
#include "typedefs.h"

inline constexpr int ENGINE_VERSION_MAJOR = 0;
inline constexpr int ENGINE_VERSION_MINOR = 21;

#ifdef NDEBUG
inline constexpr bool ENGINE_DEBUG_BUILD = false;
#else
inline constexpr bool ENGINE_DEBUG_BUILD = true;
#endif // NDEBUG

inline const std::string ENGINE_VERSION_STRING = "0.21";

inline constexpr int BLOCK_AIR = 0;
inline constexpr int ITEM_EMPTY = 0;

inline constexpr int CHUNK_W = 16;
inline constexpr int CHUNK_H = 256;
inline constexpr int CHUNK_D = 16;

inline constexpr uint VOXEL_USER_BITS = 8;
inline constexpr uint VOXEL_USER_BITS_OFFSET = sizeof(blockstate_t)*8-VOXEL_USER_BITS;

inline constexpr int ITEM_ICON_SIZE = 48;

/* Chunk volume (count of voxels per Chunk) */
inline constexpr int CHUNK_VOL = (CHUNK_W * CHUNK_H * CHUNK_D);

/* BLOCK_VOID is block id used to mark non-existing voxel (voxel of missing chunk) */
inline constexpr blockid_t BLOCK_VOID = std::numeric_limits<blockid_t>::max();
inline constexpr itemid_t ITEM_VOID = std::numeric_limits<itemid_t>::max();

inline constexpr blockid_t MAX_BLOCKS = BLOCK_VOID;

inline constexpr uint vox_index(uint x, uint y, uint z, uint w=CHUNK_W, uint d=CHUNK_D) {
    return (y * d + z) * w + x;
}

inline const std::string SHADERS_FOLDER = "shaders";
inline const std::string TEXTURES_FOLDER = "textures";
inline const std::string FONTS_FOLDER = "fonts";
inline const std::string LAYOUTS_FOLDER = "layouts";
inline const std::string SOUNDS_FOLDER = "sounds";

#endif // SRC_CONSTANTS_H_
