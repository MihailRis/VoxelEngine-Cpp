#include "ChunksStorage.h"

#include <assert.h>

#include "VoxelsVolume.h"
#include "Chunk.h"
#include "../voxmaths.h"
#include "../lighting/Lightmap.h"


using glm::ivec2;
using std::shared_ptr;

ChunksStorage::ChunksStorage() {
}

ChunksStorage::~ChunksStorage() {
}

void ChunksStorage::store(shared_ptr<Chunk> chunk) {
	chunksMap[ivec2(chunk->x, chunk->z)] = chunk;
}

shared_ptr<Chunk> ChunksStorage::get(int x, int z) const {
	auto found = chunksMap.find(ivec2(x, z));
	if (found == chunksMap.end()) {
		return nullptr;
	}
	return found->second;
}

// some magic code
void ChunksStorage::getVoxels(VoxelsVolume* volume) const {
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
			auto found = chunksMap.find(ivec2(cx, cz));
			if (found == chunksMap.end()) {
				// no chunk loaded -> filling with BLOCK_VOID
				for (int ly = y; ly < y + h; ly++) {
					for (int lz = max(z, cz * CHUNK_D);
						lz < min(z + d, (cz + 1) * CHUNK_D);
						lz++) {
						for (int lx = max(x, cx * CHUNK_W);
							lx < min(x + w, (cx + 1) * CHUNK_W);
							lx++) {
							voxels[vox_index(lx - x, ly - y, lz - z, w, d)].id = BLOCK_VOID;
							lights[vox_index(lx - x, ly - y, lz - z, w, d)] = 0;
						}
					}
				}
			}
			else {
				const std::shared_ptr<Chunk>& chunk = found->second;
				const voxel* cvoxels = chunk->voxels;
				const light_t* clights = chunk->lightmap->getLights();
				for (int ly = y; ly < y + h; ly++) {
					for (int lz = max(z, cz * CHUNK_D);
						lz < min(z + d, (cz + 1) * CHUNK_D);
						lz++) {
						for (int lx = max(x, cx * CHUNK_W);
							lx < min(x + w, (cx + 1) * CHUNK_W);
							lx++) {
							voxels[vox_index(lx - x, ly - y, lz - z, w, d)] =
								cvoxels[vox_index(lx - cx * CHUNK_W, ly, lz - cz * CHUNK_D, CHUNK_W, CHUNK_D)];
							lights[vox_index(lx - x, ly - y, lz - z, w, d)] =
								clights[vox_index(lx - cx * CHUNK_W, ly, lz - cz * CHUNK_D, CHUNK_W, CHUNK_D)];
						}
					}
				}
			}
		}
	}
}