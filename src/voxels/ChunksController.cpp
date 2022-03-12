#include "ChunksController.h"
#include "Chunk.h"
#include "Chunks.h"
#include "WorldGenerator.h"
#include "../graphics/Mesh.h"
#include "../graphics/VoxelRenderer.h"
#include "../lighting/Lighting.h"
#include "../files/WorldFiles.h"
#include "ChunksLoader.h"
#include <iostream>

#ifdef _WIN32
#define _WIN32_WINNT 0x0501
#include <mingw.thread.h>
#else
#include <thread>
#endif

#define MIN_SURROUNDING 9


ChunksController::ChunksController(Chunks* chunks, Lighting* lighting) : chunks(chunks), lighting(lighting){
	loadersCount = std::thread::hardware_concurrency() - 1;
	if (loadersCount <= 0)
		loadersCount = 1;
	loaders = new ChunksLoader*[loadersCount];
	for (int i = 0; i < loadersCount; i++){
		loaders[i] = new ChunksLoader();
	}
	std::cout << "created " << loadersCount << " loaders" << std::endl;
}

ChunksController::~ChunksController(){
	for (int i = 0; i < loadersCount; i++)
		delete loaders[i];
	delete[] loaders;
}

int ChunksController::countFreeLoaders(){
	int count = 0;
	for (int i = 0; i < loadersCount; i++){
		if (!loaders[i]->isBusy())
			count++;
	}
	return count;
}

bool ChunksController::loadVisible(WorldFiles* worldFiles){
	const int w = chunks->w;
	const int h = chunks->h;
	const int d = chunks->d;
	const int ox = chunks->ox;
	const int oy = chunks->oy;
	const int oz = chunks->oz;
	int nearX = 0;
	int nearY = 0;
	int nearZ = 0;
	int minDistance = (w/2)*(w/2);
	for (int y = 0; y < h; y++){
		for (int z = 2; z < d-2; z++){
			for (int x = 2; x < w-2; x++){
				int index = (y * d + z) * w + x;
				Chunk* chunk = chunks->chunks[index];
				if (chunk != nullptr){
					int surrounding = 0;
					for (int oz = -1; oz <= 1; oz++){
						for (int ox = -1; ox <= 1; ox++){
							Chunk* other = chunks->getChunk(chunk->x+ox, chunk->y, chunk->z+oz);
							if (other != nullptr && other->ready) surrounding++;
						}
					}
					chunk->surrounding = surrounding;
					continue;
				}
				int lx = x - w / 2;
				int ly = y - h / 2;
				int lz = z - d / 2;
				int distance = (lx * lx + ly * ly + lz * lz);
				if (distance < minDistance){
					minDistance = distance;
					nearX = x;
					nearY = y;
					nearZ = z;
				}
			}
		}
	}

	int index = (nearY * d + nearZ) * w + nearX;
	Chunk* chunk = chunks->chunks[index];
	if (chunk != nullptr)
		return false;

	ChunksLoader* freeLoader = nullptr;
	for (int i = 0; i < loadersCount; i++){
		ChunksLoader* loader = loaders[i];
		if (loader->isBusy()){
			continue;
		}
		freeLoader = loader;
		break;
	}
	if (freeLoader == nullptr)
		return false;
	chunk = new Chunk(nearX+ox,nearY+oy,nearZ+oz);
	if (worldFiles->getChunk(chunk->x, chunk->z, (char*)chunk->voxels))
		chunk->loaded = true;

	chunks->chunks[index] = chunk;

	Chunk* closes[27];
	for (int i = 0; i < 27; i++)
		closes[i] = nullptr;
	for (size_t j = 0; j < chunks->volume; j++){
		Chunk* other = chunks->chunks[j];
		if (other == nullptr)
			continue;
		if (!other->ready)
			continue;

		int ox = other->x - chunk->x;
		int oy = other->y - chunk->y;
		int oz = other->z - chunk->z;

		if (abs(ox) > 1 || abs(oy) > 1 || abs(oz) > 1)
			continue;

		ox += 1;
		oy += 1;
		oz += 1;
		closes[(oy * 3 + oz) * 3 + ox] = other;
	}
	freeLoader->perform(chunk, (Chunk**)closes);
	return true;
}

bool ChunksController::_buildMeshes(VoxelRenderer* renderer, int tick) {
	const int w = chunks->w;
	const int h = chunks->h;
	const int d = chunks->d;

	int nearX = 0;
	int nearY = 0;
	int nearZ = 0;
	int minDistance = 1000000000;
	for (int y = 0; y < h; y++){
		for (int z = 1; z < d-1; z++){
			for (int x = 1; x < w-1; x++){
				int index = (y * d + z) * w + x;
				Chunk* chunk = chunks->chunks[index];
				if (chunk == nullptr)
					continue;
				Mesh* mesh = chunks->meshes[index];
				if (mesh != nullptr && !chunk->modified)
					continue;
				if (!chunk->ready || chunk->surrounding < MIN_SURROUNDING){
					continue;
				}
				int lx = x - w / 2;
				int ly = y - h / 2;
				int lz = z - d / 2;
				int distance = (lx * lx + ly * ly + lz * lz);
				if (distance < minDistance){
					minDistance = distance;
					nearX = x;
					nearY = y;
					nearZ = z;
				}
			}
		}
	}

	int index = (nearY * d + nearZ) * w + nearX;


	Chunk* chunk = chunks->chunks[index];
	if (chunk == nullptr){
		return false;
	}
	Mesh* mesh = chunks->meshes[index];
	if (mesh == nullptr || chunk->modified){
		Chunk* closes[27];
		if (mesh != nullptr)
			delete mesh;
		if (chunk->isEmpty()){
			chunks->meshes[index] = nullptr;
			return false;
		}
		chunk->modified = false;
		for (int i = 0; i < 27; i++)
			closes[i] = nullptr;
		for (size_t j = 0; j < chunks->volume; j++){
			Chunk* other = chunks->chunks[j];
			if (other == nullptr)
				continue;
			if (!other->ready)
				continue;

			int ox = other->x - chunk->x;
			int oy = other->y - chunk->y;
			int oz = other->z - chunk->z;

			if (abs(ox) > 1 || abs(oy) > 1 || abs(oz) > 1)
				continue;

			ox += 1;
			oy += 1;
			oz += 1;
			closes[(oy * 3 + oz) * 3 + ox] = other;
		}
		mesh = renderer->render(chunk, (const Chunk**)closes);
		chunks->meshes[index] = mesh;
		return true;
	}
	return false;
}
