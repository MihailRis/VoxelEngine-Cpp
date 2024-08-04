#ifndef VOXELS_VOXEL_HPP_
#define VOXELS_VOXEL_HPP_

#include "../typedefs.hpp"

inline constexpr int BLOCK_DIR_NORTH = 0x0;
inline constexpr int BLOCK_DIR_WEST = 0x1;
inline constexpr int BLOCK_DIR_SOUTH = 0x2;
inline constexpr int BLOCK_DIR_EAST = 0x3;
inline constexpr int BLOCK_DIR_UP = 0x4;
inline constexpr int BLOCK_DIR_DOWN = 0x5;

struct blockstate {
    uint8_t rotation : 3;  // block rotation index
    uint8_t segment : 3;   // segment block bits
    uint8_t reserved : 2;  // reserved bits
    uint8_t userbits : 8;  // bits for use in block script
};
static_assert(sizeof(blockstate) == 2);

/// @brief blockstate cast to an integer (optimized out in most cases)
inline constexpr blockstate_t blockstate2int(blockstate b) {
    return static_cast<blockstate_t>(b.rotation) |
           static_cast<blockstate_t>(b.segment) << 3 |
           static_cast<blockstate_t>(b.reserved) << 6 |
           static_cast<blockstate_t>(b.userbits) << 8;
}

/// @brief integer cast to a blockstate (optimized out in most cases)
inline constexpr blockstate int2blockstate(blockstate_t i) {
    return {
        static_cast<uint8_t>(i & 0b111),
        static_cast<uint8_t>((i >> 3) & 0b111),
        static_cast<uint8_t>((i >> 6) & 0b11),
        static_cast<uint8_t>((i >> 8) & 0xFF)};
}

struct voxel {
    blockid_t id;
    blockstate state;
};
static_assert(sizeof(voxel) == 4);

#endif  // VOXELS_VOXEL_HPP_
