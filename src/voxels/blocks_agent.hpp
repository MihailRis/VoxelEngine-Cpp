#pragma once

#include "voxel.hpp"
#include "typedefs.hpp"
#include "maths/voxmaths.hpp"

class Chunk;
class Chunks;
class GlobalChunks;

/// Using templates to minimize OOP overhead

namespace blocks_agent {

template<class Storage>
inline Chunk* get_chunk(const Storage& chunks, int cx, int cz) {
    return chunks.getChunk(cx, cz);
}

template<class Storage>
inline voxel* get(const Storage& chunks, int32_t x, int32_t y, int32_t z) {
    if (y < 0 || y >= CHUNK_H) {
        return nullptr;
    }
    int cx = floordiv<CHUNK_W>(x);
    int cz = floordiv<CHUNK_D>(z);
    Chunk* chunk = get_chunk(chunks, cx, cz);
    if (chunk == nullptr) {
        return nullptr;
    }
    int lx = x - cx * CHUNK_W;
    int lz = z - cz * CHUNK_D;
    return &chunk->voxels[(y * CHUNK_D + lz) * CHUNK_W + lx];
}

template<class Storage>
inline voxel& require(const Storage& chunks, int32_t x, int32_t y, int32_t z) {
    auto vox = get(chunks, x, y, z);
    if (vox == nullptr) {
        throw std::runtime_error("voxel does not exist");
    }
    return *vox;
}

template<class Storage>
inline const Block& get_block_def(const Storage& chunks, blockid_t id) {
    return chunks.getContentIndices().blocks.require(id);
}

template<class Storage>
inline bool is_solid_at(const Storage& chunks, int32_t x, int32_t y, int32_t z) {
    if (auto vox = get(chunks, x, y, z)) {
        return get_block_def(chunks, vox->id).rt.solid;
    }
    return false;
}

} // blocks_agent
