#include "ChunksLoader.h"
#include <chrono>

#include "Chunk.h"
#include "Chunks.h"
#include "../world/World.h"
#include "WorldGenerator.h"
#include "../lighting/Lighting.h"
#include "../graphics/VoxelRenderer.h"

#include <iostream>

#define CLOSES_C 27

void ChunksLoader::_thread(){
	Chunks chunks(3,3,3, -1,-1,-1);
	Lighting lighting(&chunks);
	VoxelRenderer renderer;
	while (state != OFF){
		if (current == nullptr){
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			continue;
		}
		Chunk* chunk = current;
		chunks._setOffset(chunk->x-1, chunk->y-1, chunk->z-1);
		for (size_t i = 0; i < CLOSES_C; i++){
			Chunk* other = closes[i];
			if (other){
				chunks.putChunk(other);
			}
		}

		if (state == LOAD){
			chunks.putChunk(chunk);
			if (!chunk->loaded){
				WorldGenerator::generate(chunk->voxels, chunk->x, chunk->y, chunk->z, world.load()->seed);
			}

			lighting.onChunkLoaded(chunk->x, chunk->y, chunk->z, true);
		}
		else if (state == RENDER){
			size_t size;
			renderer.render(chunk, (const Chunk**)(closes.load()), size);
			float* vertices = new float[size];
			for (size_t i = 0; i < size; i++)
				vertices[i] = renderer.buffer[i];
			chunk->renderData.vertices = vertices;
			chunk->renderData.size = size;
		}

		chunks.clear(false);
		for (int i = 0; i < CLOSES_C; i++){
			Chunk* other = closes[i];
			if (other)
				other->decref();
		}
		chunk->ready = true;
		current = nullptr;
		chunk->decref();
	}
}

void ChunksLoader::perform(Chunk* chunk, Chunk** closes_passed, LoaderMode mode){
	if (isBusy()){
		std::cerr << "performing while busy" << std::endl;
		return;
	}
	chunk->incref();
	if (closes == nullptr){
		closes = new Chunk*[CLOSES_C];
	}
	for (int i = 0; i < CLOSES_C; i++){
		Chunk* other = closes_passed[i];
		if (other == nullptr)
			closes[i] = nullptr;
		else {
			other->incref();
			closes[i] = other;
		}
	}
	current = chunk;
	state = mode;
}

void ChunksLoader::load(Chunk* chunk, Chunk** closes_passed){
	perform(chunk, closes_passed, LOAD);
}

void ChunksLoader::render(Chunk* chunk, Chunk** closes_passed){
	perform(chunk, closes_passed, RENDER);
}
