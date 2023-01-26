#include "ChunksLoader.h"
#include <chrono>

#include "Chunk.h"
#include "Chunks.h"
#include "Block.h"
#include "voxel.h"
#include "../world/World.h"
#include "WorldGenerator.h"
#include "../lighting/Lighting.h"
#include "../graphics/VoxelRenderer.h"

#include <iostream>

#define SURROUNDINGS_C 9

void ChunksLoader::_thread(){
	Chunks chunks(3, 3, -1, -1);
	Lighting lighting(&chunks);
	VoxelRenderer renderer;
	while (state != OFF){
		if (current == nullptr){
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			continue;
		}
		Chunk* chunk = current;
		chunks._setOffset(chunk->x-1, chunk->z-1);
		for (size_t i = 0; i < SURROUNDINGS_C; i++){
			Chunk* other = surroundings[i];
			if (other){
				chunks.putChunk(other);
			}
		}

		if (state == LOAD){
			chunks.putChunk(chunk);
			if (!chunk->isLoaded()){
				WorldGenerator::generate(chunk->voxels, chunk->x, chunk->z, world->seed);
				chunk->setUnsaved(true);
			}

			for (size_t i = 0; i < CHUNK_VOL; i++){
				if (Block::blocks[chunk->voxels[i].id] == nullptr){
					std::cout << "corruped block detected at " << i << " of chunk " << chunk->x << "x" << chunk->z << std::endl;
					chunk->voxels[i].id = 11;
				}
			}
			lighting.prebuildSkyLight(chunk->x, chunk->z);
		}
		else if (state == LIGHTS) {
			lighting.buildSkyLight(chunk->x, chunk->z);
			lighting.onChunkLoaded(chunk->x, chunk->z);
			chunk->setLighted(true);
		}
		else if (state == RENDER){
			chunk->setModified(false);
			size_t size;
			renderer.render(chunk, (const Chunk**)(surroundings.load()), size);
			float* vertices = new float[size];
			for (size_t i = 0; i < size; i++)
				vertices[i] = renderer.buffer[i];
			chunk->renderData.vertices = vertices;
			chunk->renderData.size = size;
		}

		chunks.clear(false);
		for (int i = 0; i < SURROUNDINGS_C; i++){
			Chunk* other = surroundings[i];
			if (other)
				other->decref();
		}
		chunk->setReady(true);
		current = nullptr;
		chunk->decref();
	}
}

void ChunksLoader::perform(Chunk* chunk, Chunk** surroundings_passed, LoaderMode mode){
	if (isBusy()){
		std::cerr << "performing while busy" << std::endl;
		return;
	}
	chunk->incref();
	if (surroundings == nullptr){
		surroundings = new Chunk*[SURROUNDINGS_C];
	}
	for (int i = 0; i < SURROUNDINGS_C; i++){
		Chunk* other = surroundings_passed[i];
		if (other == nullptr)
			surroundings[i] = nullptr;
		else {
			other->incref();
			surroundings[i] = other;
		}
	}
	current = chunk;
	state = mode;
}

void ChunksLoader::load(Chunk* chunk, Chunk** closes_passed){
	perform(chunk, closes_passed, LOAD);
}

void ChunksLoader::lights(Chunk* chunk, Chunk** closes_passed){
	perform(chunk, closes_passed, LIGHTS);
}

void ChunksLoader::render(Chunk* chunk, Chunk** closes_passed){
	perform(chunk, closes_passed, RENDER);
}
