#include "blocks_agent.hpp"

using namespace blocks_agent;

template <class Storage>
static inline void set_block(
    Storage& chunks,
    int32_t x,
    int32_t y,
    int32_t z,
    uint32_t id,
    blockstate state
) {
    if (y < 0 || y >= CHUNK_H) {
        return;
    }
    const auto& indices = chunks.getContentIndices();
    int cx = floordiv<CHUNK_W>(x);
    int cz = floordiv<CHUNK_D>(z);
    Chunk* chunk = get_chunk(chunks, cx, cz);
    if (chunk == nullptr) {
        return;
    }
    int lx = x - cx * CHUNK_W;
    int lz = z - cz * CHUNK_D;
    size_t index = vox_index(lx, y, lz);

    // block finalization
    voxel& vox = chunk->voxels[(y * CHUNK_D + lz) * CHUNK_W + lx];
    const auto& prevdef = indices.blocks.require(vox.id);
    if (prevdef.inventorySize != 0) {
        chunk->removeBlockInventory(lx, y, lz);
    }
    if (prevdef.rt.extended && !vox.state.segment) {
        erase_segments(chunks, prevdef, vox.state, x, y, z);
    }
    if (prevdef.dataStruct) {
        if (auto found = chunk->blocksMetadata.find(index)) {
            chunk->blocksMetadata.free(found);
            chunk->flags.unsaved = true;
            chunk->flags.blocksData = true;
        }
    }

    // block initialization
    const auto& newdef = indices.blocks.require(id);
    vox.id = id;
    vox.state = state;
    chunk->setModifiedAndUnsaved();
    if (!state.segment && newdef.rt.extended) {
        repair_segments(chunks, newdef, state, x, y, z);
    }

    if (y < chunk->bottom)
        chunk->bottom = y;
    else if (y + 1 > chunk->top)
        chunk->top = y + 1;
    else if (id == 0)
        chunk->updateHeights();

    if (lx == 0 && (chunk = get_chunk(chunks, cx - 1, cz))) {
        chunk->flags.modified = true;
    }
    if (lz == 0 && (chunk = get_chunk(chunks, cx, cz - 1))) {
        chunk->flags.modified = true;
    }
    if (lx == CHUNK_W - 1 && (chunk = get_chunk(chunks, cx + 1, cz))) {
        chunk->flags.modified = true;
    }
    if (lz == CHUNK_D - 1 && (chunk = get_chunk(chunks, cx, cz + 1))) {
        chunk->flags.modified = true;
    }
}

void blocks_agent::set(
    Chunks& chunks,
    int32_t x,
    int32_t y,
    int32_t z,
    uint32_t id,
    blockstate state
) {
    set_block(chunks, x, y, z, id, state);
}

void blocks_agent::set(
    GlobalChunks& chunks,
    int32_t x,
    int32_t y,
    int32_t z,
    uint32_t id,
    blockstate state
) {
    set_block(chunks, x, y, z, id, state);
}
