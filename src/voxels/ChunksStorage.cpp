#include "ChunksStorage.h"

#include <assert.h>
#include <iostream>

#include "VoxelsVolume.h"
#include "Chunk.h"
#include "Block.h"
#include "../content/Content.h"
#include "../files/WorldFiles.h"
#include "../world/Level.h"
#include "../world/World.h"
#include "../maths/voxmaths.h"
#include "../lighting/Lightmap.h"
#include "../typedefs.h"

ChunksStorage::ChunksStorage(Level* level) : level(level) {
}

void ChunksStorage::store(std::shared_ptr<Chunk> chunk) {
	chunksMap[glm::ivec2(chunk->x, chunk->z)] = chunk;
}

std::shared_ptr<Chunk> ChunksStorage::get(int x, int z) const {
	auto found = chunksMap.find(glm::ivec2(x, z));
	if (found == chunksMap.end()) {
		return nullptr;
	}
	return found->second;
}

void ChunksStorage::remove(int x, int z) {
	auto found = chunksMap.find(glm::ivec2(x, z));
	if (found != chunksMap.end()) {
		chunksMap.erase(found->first);
	}
}

static void verifyLoadedChunk(ContentIndices* indices, Chunk* chunk) {
    for (size_t i = 0; i < CHUNK_VOL; i++) {
        blockid_t id = chunk->voxels[i].id;
        if (indices->getBlockDef(id) == nullptr) {
            std::cout << "corruped block detected at " << i << " of chunk ";
            std::cout << chunk->x << "x" << chunk->z;
            std::cout << " -> " << (int)id << std::endl;
            chunk->voxels[i].id = 11;
        }
    }
}

std::shared_ptr<Chunk> ChunksStorage::create(int x, int z) {
	World* world = level->getWorld();
    WorldFiles* wfile = world->wfile;

    auto chunk = std::make_shared<Chunk>(x, z);
	store(chunk);
	std::unique_ptr<ubyte[]> data(wfile->getChunk(chunk->x, chunk->z));
	if (data) {
		chunk->decode(data.get());
		auto invs = wfile->fetchInventories(chunk->x, chunk->z);
		chunk->setBlockInventories(std::move(invs));
		chunk->setLoaded(true);
        verifyLoadedChunk(level->content->getIndices(), chunk.get());
	}

	std::unique_ptr<light_t[]> lights (wfile->getLights(chunk->x, chunk->z));
	if (lights) {
		chunk->lightmap.set(lights.get());
		chunk->setLoadedLights(true);
	}
	return chunk;
}

// some magic code
void ChunksStorage::getVoxels(VoxelsVolume* volume, bool backlight) const {
	const Content* content = level->content;
	auto indices = content->getIndices();
	voxel* voxels = volume->getVoxels();
	light_t* lights = volume->getLights();
	int x = volume->getX();
	int y = volume->getY();
	int z = volume->getZ();

	int w = volume->getW();
	int h = volume->getH();
	int d = volume->getD();

	int scx = floordiv(x, CHUNK_W);
	int scz = floordiv(z, CHUNK_D);

	int ecx = floordiv(x + w, CHUNK_W);
	int ecz = floordiv(z + d, CHUNK_D);

	int cw = ecx - scx + 1;
	int ch = ecz - scz + 1;

	// cw*ch chunks will be scanned
	for (int cz = scz; cz < scz + ch; cz++) {
		for (int cx = scx; cx < scx + cw; cx++) {
			auto found = chunksMap.find(glm::ivec2(cx, cz));
			if (found == chunksMap.end()) {
				// no chunk loaded -> filling with BLOCK_VOID
				for (int ly = y; ly < y + h; ly++) {
					for (int lz = max(z, cz * CHUNK_D);
						lz < min(z + d, (cz + 1) * CHUNK_D);
						lz++) {
						for (int lx = max(x, cx * CHUNK_W);
							lx < min(x + w, (cx + 1) * CHUNK_W);
							lx++) {
							uint idx = vox_index(lx - x, ly - y, lz - z, w, d);
							voxels[idx].id = BLOCK_VOID;
							lights[idx] = 0;
						}
					}
				}
			} else {
				auto& chunk = found->second;
				const voxel* cvoxels = chunk->voxels;
				const light_t* clights = chunk->lightmap.getLights();
				for (int ly = y; ly < y + h; ly++) {
					for (int lz = max(z, cz * CHUNK_D);
						lz < min(z + d, (cz + 1) * CHUNK_D);
						lz++) {
						for (int lx = max(x, cx * CHUNK_W);
							lx < min(x + w, (cx + 1) * CHUNK_W);
							lx++) {
							uint vidx = vox_index(lx - x, ly - y, lz - z, w, d);
							uint cidx = vox_index(lx - cx * CHUNK_W, ly, 
										lz - cz * CHUNK_D, CHUNK_W, CHUNK_D);
							voxels[vidx] = cvoxels[cidx];
							light_t light = clights[cidx];
							if (backlight) {
								const Block* block = indices->getBlockDef(voxels[vidx].id);
								if (block->lightPassing) {
									light = Lightmap::combine(
										min(15, Lightmap::extract(light, 0)+1),
										min(15, Lightmap::extract(light, 1)+1),
										min(15, Lightmap::extract(light, 2)+1),
										min(15, Lightmap::extract(light, 3))
									);
								}
							}
							lights[vidx] = light;
						}
					}
				}
			}
		}
	}
}
