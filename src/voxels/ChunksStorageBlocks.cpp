#include "Chunk.h"
#include "ChunksStorage.h"
#include "../constants.h"
#include "../maths/voxmaths.h"

Chunk* ChunksStorage::getChunk(int x, int z) const {
	auto found = chunksMap.find(glm::ivec2(x, z));
	if (found == chunksMap.end()) {
		return nullptr;
	}
	return found->second.get();
}

Chunk* ChunksStorage::getChunkByVoxel(int32_t x, int32_t y, int32_t z) const {
	if (y < 0 || y >= CHUNK_H)
		return nullptr;
	int32_t cx = floordiv(x, CHUNK_W);
	int32_t cz = floordiv(z, CHUNK_D);
	return getChunk(cx, cz);
}

voxel* ChunksStorage::getVoxel(int32_t x, int32_t y, int32_t z) const {
	if (y < 0 || y >= CHUNK_H)
		return nullptr;
	int32_t cx = floordiv(x, CHUNK_W);
	int32_t cy = floordiv(y, CHUNK_H);
	int32_t cz = floordiv(z, CHUNK_D);
	auto chunk = getChunk(cx, cz);
    if (chunk == nullptr)
        return nullptr;
    int32_t lx = x - cx * CHUNK_W;
    int32_t ly = y - cy * CHUNK_H;
    int32_t lz = z - cz * CHUNK_D;
	return &chunk->voxels[(ly * CHUNK_D + lz) * CHUNK_W + lx];
}

void ChunksStorage::setVoxel(int32_t x, int32_t y, int32_t z, blockid_t id, uint8_t states) {
	if (y < 0 || y >= CHUNK_H) return;
	int32_t cx = floordiv(x, CHUNK_W);
	int32_t cy = floordiv(y, CHUNK_H);
	int32_t cz = floordiv(z, CHUNK_D);
	auto chunk = getChunk(cx, cz);
    if (chunk == nullptr) return;
    int32_t lx = x - cx * CHUNK_W;
    int32_t ly = y - cy * CHUNK_H;
    int32_t lz = z - cz * CHUNK_D;

    voxel& vox = chunk->voxels[(ly * CHUNK_D + lz) * CHUNK_W + lx]; 
    chunk->removeBlockInventory(lx, ly, lz);
	vox.id = id;
	vox.states = states;

	chunk->setUnsaved(true);
	chunk->setModified(true);

	if (y < chunk->bottom) chunk->bottom = y;
	else if (y + 1 > chunk->top) chunk->top = y + 1;
	else if (id == 0) chunk->updateHeights();

	if (lx == 0 && (chunk = getChunk(cx - 1, cz)))
		chunk->setModified(true);
	if (lz == 0 && (chunk = getChunk(cx, cz - 1))) 
		chunk->setModified(true);

	if (lx == CHUNK_W - 1 && (chunk = getChunk(cx + 1, cz ))) 
		chunk->setModified(true);
	if (lz == CHUNK_D - 1 && (chunk = getChunk(cx, cz + 1))) 
		chunk->setModified(true);
}

ubyte ChunksStorage::getLight(int32_t x, int32_t y, int32_t z, int channel) {
	if (y < 0 || y >= CHUNK_H)
		return 0;
	int32_t cx = floordiv(x, CHUNK_W);
	int32_t cy = floordiv(y, CHUNK_H);
	int32_t cz = floordiv(z, CHUNK_D);
	auto chunk = getChunk(cx, cz);
    if (chunk == nullptr)
        return 0;
    int32_t lx = x - cx * CHUNK_W;
    int32_t ly = y - cy * CHUNK_H;
    int32_t lz = z - cz * CHUNK_D;
	return chunk->lightmap.get(lx, ly, lz, channel);
}

light_t ChunksStorage::getLight(int32_t x, int32_t y, int32_t z) {
	if (y < 0 || y >= CHUNK_H)
		return 0;
	int32_t cx = floordiv(x, CHUNK_W);
	int32_t cy = floordiv(y, CHUNK_H);
	int32_t cz = floordiv(z, CHUNK_D);
	auto chunk = getChunk(cx, cz);
    if (chunk == nullptr)
        return 0;
    int32_t lx = x - cx * CHUNK_W;
    int32_t ly = y - cy * CHUNK_H;
    int32_t lz = z - cz * CHUNK_D;
	return chunk->lightmap.get(lx,ly,lz);
}

