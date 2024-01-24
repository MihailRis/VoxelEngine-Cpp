#include <iostream>
#include <assert.h>
#include "LightSolver.h"
#include "Lightmap.h"
#include "../content/Content.h"
#include "../voxels/Chunks.h"
#include "../voxels/Chunk.h"
#include "../voxels/voxel.h"
#include "../voxels/Block.h"

LightSolver::LightSolver(const ContentIndices* contentIds, Chunks* chunks, int channel) 
	: contentIds(contentIds), 
	  chunks(chunks), 
	  channel(channel) {
}

void LightSolver::add(int x, int y, int z, int emission) {
	if (emission <= 1)
		return;

	addqueue.push(lightentry {x, y, z, ubyte(emission)});

	Chunk* chunk = chunks->getChunkByVoxel(x, y, z);
	chunk->setModified(true);
	chunk->lightmap->set(x-chunk->x*CHUNK_W, y, z-chunk->z*CHUNK_D, channel, emission);
}

void LightSolver::add(int x, int y, int z) {
	assert (chunks != nullptr);
	add(x,y,z, chunks->getLight(x,y,z, channel));
}

void LightSolver::remove(int x, int y, int z) {
	Chunk* chunk = chunks->getChunkByVoxel(x, y, z);
	if (chunk == nullptr)
		return;

	ubyte light = chunk->lightmap->get(x-chunk->x*CHUNK_W, y, z-chunk->z*CHUNK_D, channel);
	if (light == 0){
		return;
	}
	remqueue.push(lightentry {x, y, z, light});
	chunk->lightmap->set(x-chunk->x*CHUNK_W, y, z-chunk->z*CHUNK_D, channel, 0);
}

void LightSolver::solve(){
	const int coords[] = {
			0, 0, 1,
			0, 0,-1,
			0, 1, 0,
			0,-1, 0,
			1, 0, 0,
		   -1, 0, 0
	};

	while (!remqueue.empty()){
		const lightentry entry = remqueue.front();
		remqueue.pop();

		for (int i = 0; i < 6; i++) {
			int x = entry.x+coords[i*3+0];
			int y = entry.y+coords[i*3+1];
			int z = entry.z+coords[i*3+2];
			Chunk* chunk = chunks->getChunkByVoxel(x,y,z);
			if (chunk) {
				int lx = x - chunk->x * CHUNK_W;
				int lz = z - chunk->z * CHUNK_D;
				chunk->setModified(true);

				ubyte light = chunk->lightmap->get(lx,y,lz, channel);
				if (light != 0 && light == entry.light-1){
					remqueue.push(lightentry {x, y, z, light});
					chunk->lightmap->set(lx, y, lz, channel, 0);
				}
				else if (light >= entry.light){
					addqueue.push(lightentry {x, y, z, light});
				}
			}
		}
	}

	const Block* const* blockDefs = contentIds->getBlockDefs();
	while (!addqueue.empty()){
		const lightentry entry = addqueue.front();
		addqueue.pop();

		for (int i = 0; i < 6; i++) {
			int x = entry.x+coords[i*3+0];
			int y = entry.y+coords[i*3+1];
			int z = entry.z+coords[i*3+2];
			Chunk* chunk = chunks->getChunkByVoxel(x,y,z);
			if (chunk) {
				int lx = x - chunk->x * CHUNK_W;
				int lz = z - chunk->z * CHUNK_D;
				chunk->setModified(true);

				ubyte light = chunk->lightmap->get(lx, y, lz, channel);
				voxel& v = chunk->voxels[vox_index(lx, y, lz)];
				const Block* block = blockDefs[v.id];
				if (block->lightPassing && light+2 <= entry.light){
					chunk->lightmap->set(
						x-chunk->x*CHUNK_W, y, z-chunk->z*CHUNK_D, 
						channel, 
						entry.light-1);
					addqueue.push(lightentry {x, y, z, ubyte(entry.light-1)});
				}
			}
		}
	}
}
