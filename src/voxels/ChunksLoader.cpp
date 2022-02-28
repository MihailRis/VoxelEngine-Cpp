#include "ChunksLoader.h"
#include <chrono>

#include "Chunk.h"
#include "Chunks.h"
#include "WorldGenerator.h"
#include "../lighting/Lighting.h"

#include <iostream>

void ChunksLoader::_thread(){
	Chunks chunks(3,3,3, -1,-1,-1);
	Lighting lighting(&chunks);
	while (working){
		if (current == nullptr){
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			continue;
		}
		Chunk* chunk = current;
		chunk->incref();

		chunks._setOffset(chunk->x-1, chunk->y-1, chunk->z-1);

		if (!chunk->generated){
			WorldGenerator::generate(chunk->voxels, chunk->x, chunk->y, chunk->z);
		}

		chunks.putChunk(chunk);
		lighting.onChunkLoaded(chunk->x, chunk->y, chunk->z, true);
		chunks.clear(false);
		for (int i = 0; i < 27; i++){
			Chunk* other = closes[i];
			//delete other;
		}
		chunk->ready = true;
		current = nullptr;
		chunk->decref();
		//std::cout << "LOADER: success" << std::endl;
	}
}

void ChunksLoader::perform(Chunk* chunk, const Chunk** cs){
	if (isBusy()){
		std::cerr << "performing while busy" << std::endl;
		return;
	}
	if (closes == nullptr){
		closes = new Chunk*[27];
	}
	for (int i = 0; i < 27; i++){
		const Chunk* other = cs[i];
		if (other == nullptr)
			closes[i] = nullptr;
		else
			closes[i] = (Chunk*)other;//->clone();
	}
	current = chunk;
}
