#include "ChunksController.h"
#include "Block.h"
#include "Chunk.h"
#include "Chunks.h"
#include "ChunksStorage.h"
#include "WorldGenerator.h"
#include "../content/Content.h"
#include "../graphics/Mesh.h"
#include "../lighting/Lighting.h"
#include "../files/WorldFiles.h"
#include "../world/Level.h"
#include "../world/World.h"
#include "../maths/voxmaths.h"
#include <iostream>
#include <limits.h>
#include <memory>
#include <chrono>

#define MAX_WORK_PER_FRAME 16
#define MIN_SURROUNDING 9

using std::unique_ptr;
using std::shared_ptr;
using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::microseconds;


ChunksController::ChunksController(Level* level, Chunks* chunks, Lighting* lighting, uint padding) 
    : level(level), 
	  chunks(chunks), 
	  lighting(lighting), 
	  padding(padding), 
	  generator(new WorldGenerator(level->content)) {
}

ChunksController::~ChunksController(){
	delete generator;
}

void ChunksController::update(int64_t maxDuration) {
    int64_t mcstotal = 0;
    for (uint i = 0; i < MAX_WORK_PER_FRAME; i++) {
        auto start = high_resolution_clock::now();
        if (loadVisible()) {
            auto elapsed = high_resolution_clock::now() - start;
            int64_t mcs = duration_cast<microseconds>(elapsed).count();
            avgDurationMcs = mcs * 0.2 + avgDurationMcs * 0.8;
            if (mcstotal + max(avgDurationMcs, mcs) * 2 < maxDuration * 1000) {
                mcstotal += mcs;
                continue;
            }
        }
        break;
    }
}

bool ChunksController::loadVisible(){
	const Content* content = level->content;
	const int w = chunks->w;
	const int d = chunks->d;
	const int ox = chunks->ox;
	const int oz = chunks->oz;
	int nearX = 0;
	int nearZ = 0;
	int minDistance = ((w-padding*2)/2)*((w-padding*2)/2);
	for (uint z = padding; z < d-padding; z++){
		for (uint x = padding; x < w-padding; x++){
			int index = z * w + x;
			shared_ptr<Chunk> chunk = chunks->chunks[index];
			if (chunk != nullptr){
				int surrounding = 0;
				for (int oz = -1; oz <= 1; oz++){
					for (int ox = -1; ox <= 1; ox++){
						Chunk* other = chunks->getChunk(chunk->x+ox, chunk->z+oz);
						if (other != nullptr) surrounding++;
					}
				}
				chunk->surrounding = surrounding;
				if (surrounding == MIN_SURROUNDING && !chunk->isLighted()) {
					lighting->buildSkyLight(chunk->x, chunk->z);
					lighting->onChunkLoaded(chunk->x, chunk->z);
					chunk->setLighted(true);
					return true;
				}
				continue;
			}
			int lx = x - w / 2;
			int lz = z - d / 2;
			int distance = (lx * lx + lz * lz);
			if (distance < minDistance){
				minDistance = distance;
				nearX = x;
				nearZ = z;
			}
		}
	}

	int index = nearZ * w + nearX;
	shared_ptr<Chunk> chunk = chunks->chunks[index];
	if (chunk != nullptr) {
		return false;
	}

	chunk = level->chunksStorage->create(nearX+ox, nearZ+oz);
	chunks->putChunk(chunk);

	if (!chunk->isLoaded()) {
		generator->generate(chunk->voxels, chunk->x, chunk->z, level->world->seed);
		chunk->setUnsaved(true);
	}

	chunk->updateHeights();

	ContentIndices* indices = content->indices;
	for (size_t i = 0; i < CHUNK_VOL; i++) {
		blockid_t id = chunk->voxels[i].id;
		if (indices->getBlockDef(id) == nullptr) {
			std::cout << "corruped block detected at " << i << " of chunk ";
			std::cout << chunk->x << "x" << chunk->z;
			std::cout << " -> " << (int)id << std::endl;
			chunk->voxels[i].id = 11;
		}
	}
	lighting->prebuildSkyLight(chunk->x, chunk->z);
	return true;
}
