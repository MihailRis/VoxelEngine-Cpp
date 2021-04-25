#include "LightSolver.h"
#include "Lightmap.h"
#include "../voxels/Chunks.h"
#include "../voxels/Chunk.h"
#include "../voxels/voxel.h"
#include "../voxels/Block.h"

LightSolver::LightSolver(Chunks* chunks, int channel) : chunks(chunks), channel(channel) {
}

void LightSolver::add(int x, int y, int z, int emission) {
	if (emission <= 1)
		return;
	lightentry entry;
	entry.x = x;
	entry.y = y;
	entry.z = z;
	entry.light = emission;
	addqueue.push(entry);

	Chunk* chunk = chunks->getChunkByVoxel(entry.x, entry.y, entry.z);
	chunk->modified = true;
	chunk->lightmap->set(entry.x-chunk->x*CHUNK_W, entry.y-chunk->y*CHUNK_H, entry.z-chunk->z*CHUNK_D, channel, entry.light);
}

void LightSolver::add(int x, int y, int z) {
	add(x,y,z, chunks->getLight(x,y,z, channel));
}

void LightSolver::remove(int x, int y, int z) {
	Chunk* chunk = chunks->getChunkByVoxel(x, y, z);
	if (chunk == nullptr)
		return;

	int light = chunk->lightmap->get(x-chunk->x*CHUNK_W, y-chunk->y*CHUNK_H, z-chunk->z*CHUNK_D, channel);
	if (light == 0){
		return;
	}

	lightentry entry;
	entry.x = x;
	entry.y = y;
	entry.z = z;
	entry.light = light;
	remqueue.push(entry);

	chunk->lightmap->set(entry.x-chunk->x*CHUNK_W, entry.y-chunk->y*CHUNK_H, entry.z-chunk->z*CHUNK_D, channel, 0);
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
		lightentry entry = remqueue.front();
		remqueue.pop();

		for (size_t i = 0; i < 6; i++) {
			int x = entry.x+coords[i*3+0];
			int y = entry.y+coords[i*3+1];
			int z = entry.z+coords[i*3+2];
			Chunk* chunk = chunks->getChunkByVoxel(x,y,z);
			if (chunk) {
				int light = chunks->getLight(x,y,z, channel);
				if (light != 0 && light == entry.light-1){
					lightentry nentry;
					nentry.x = x;
					nentry.y = y;
					nentry.z = z;
					nentry.light = light;
					remqueue.push(nentry);
					chunk->lightmap->set(x-chunk->x*CHUNK_W, y-chunk->y*CHUNK_H, z-chunk->z*CHUNK_D, channel, 0);
					chunk->modified = true;
				}
				else if (light >= entry.light){
					lightentry nentry;
					nentry.x = x;
					nentry.y = y;
					nentry.z = z;
					nentry.light = light;
					addqueue.push(nentry);
				}
			}
		}
	}

	while (!addqueue.empty()){
		lightentry entry = addqueue.front();
		addqueue.pop();

		if (entry.light <= 1)
			continue;

		for (size_t i = 0; i < 6; i++) {
			int x = entry.x+coords[i*3+0];
			int y = entry.y+coords[i*3+1];
			int z = entry.z+coords[i*3+2];
			Chunk* chunk = chunks->getChunkByVoxel(x,y,z);
			if (chunk) {
				int light = chunks->getLight(x,y,z, channel);
				voxel* v = chunks->get(x,y,z);
				Block* block = Block::blocks[v->id];
				if (block->lightPassing && light+2 <= entry.light){
					chunk->lightmap->set(x-chunk->x*CHUNK_W, y-chunk->y*CHUNK_H, z-chunk->z*CHUNK_D, channel, entry.light-1);
					chunk->modified = true;
					lightentry nentry;
					nentry.x = x;
					nentry.y = y;
					nentry.z = z;
					nentry.light = entry.light-1;
					addqueue.push(nentry);
				}
			}
		}
	}
}
