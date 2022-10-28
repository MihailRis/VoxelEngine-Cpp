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
#include <limits.h>

#ifdef _WIN32
#define _WIN32_WINNT 0x0501
#include <mingw.thread.h>
#else
#include <thread>
#endif

#define MIN_SURROUNDING 9


ChunksController::ChunksController(World* world, Chunks* chunks, Lighting* lighting) : chunks(chunks), lighting(lighting){
	loadersCount = std::thread::hardware_concurrency() * 2 - 1;
	if (loadersCount <= 0)
		loadersCount = 1;
	loaders = new ChunksLoader*[loadersCount];
	for (int i = 0; i < loadersCount; i++){
		loaders[i] = new ChunksLoader(world);
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
	const int d = chunks->d;
	const int ox = chunks->ox;
	const int oz = chunks->oz;
	int nearX = 0;
	int nearZ = 0;
	int minDistance = (w/2)*(w/2);
	for (int z = 2; z < d-2; z++){
		for (int x = 2; x < w-2; x++){
			int index = z * w + x;
			Chunk* chunk = chunks->chunks[index];
			if (chunk != nullptr){
				int surrounding = 0;
				for (int oz = -1; oz <= 1; oz++){
					for (int ox = -1; ox <= 1; ox++){
						Chunk* other = chunks->getChunk(chunk->x+ox, chunk->z+oz);
						if (other != nullptr && other->isReady()) surrounding++;
					}
				}
				chunk->surrounding = surrounding;
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
	Chunk* chunk = chunks->chunks[index];
	if (chunk != nullptr)
		return false;

	ChunksLoader* freeLoader = getFreeLoader();
	if (freeLoader == nullptr)
		return false;

	chunk = new Chunk(nearX+ox, nearZ+oz);
	if (worldFiles->getChunk(chunk->x, chunk->z, (char*)chunk->voxels))
		chunk->setLoaded(true);

	chunks->putChunk(chunk);

	Chunk* closes[9];
	for (int i = 0; i < 9; i++)
		closes[i] = nullptr;
	for (size_t j = 0; j < chunks->volume; j++){
		Chunk* other = chunks->chunks[j];
		if (other == nullptr)
			continue;
		if (!other->isReady())
			continue;

		int ox = other->x - chunk->x;
		int oz = other->z - chunk->z;

		if (abs(ox) > 1 || abs(oz) > 1)
			continue;

		ox += 1;
		oz += 1;
		closes[oz * 3 + ox] = other;
	}
	freeLoader->load(chunk, (Chunk**)closes);
	return true;
}

ChunksLoader* ChunksController::getFreeLoader() {
	ChunksLoader* freeLoader = nullptr;
	for (int i = 0; i < loadersCount; i++){
		ChunksLoader* loader = loaders[i];
		if (loader->isBusy()){
			continue;
		}
		freeLoader = loader;
		break;
	}
	return freeLoader;
}

void ChunksController::calculateLights() {
	ChunksLoader* freeLoader = getFreeLoader();
	if (freeLoader == nullptr)
		return;
	const int w = chunks->w;
	const int d = chunks->d;
	int nearX = 0;
	int nearZ = 0;
	int minDistance = INT_MAX;
	for (int z = 1; z < d-1; z++){
		for (int x = 1; x < w-1; x++){
			int index = z * w + x;
			Chunk* chunk = chunks->chunks[index];
			if (chunk == nullptr)
				continue;
			if (chunk->isLighted() || chunk->surrounding < MIN_SURROUNDING){
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
	Chunk* chunk = chunks->chunks[index];
	if (chunk == nullptr)
		return;
	Chunk* closes[9];
	for (int i = 0; i < 9; i++)
		closes[i] = nullptr;
	for (size_t j = 0; j < chunks->volume; j++){
		Chunk* other = chunks->chunks[j];
		if (other == nullptr)
			continue;

		int ox = other->x - chunk->x;
		int oz = other->z - chunk->z;

		if (abs(ox) > 1|| abs(oz) > 1)
			continue;

		ox += 1;
		oz += 1;
		closes[oz * 3 + ox] = other;
	}
	freeLoader->lights(chunk, (Chunk**)closes);
}

bool ChunksController::_buildMeshes(VoxelRenderer* renderer, int tick) {
	const int w = chunks->w;
	const int d = chunks->d;

	for (int z = 1; z < d-1; z++){
		for (int x = 1; x < w-1; x++){
			int index = z * w + x;
			Chunk* chunk = chunks->chunks[index];
			if (chunk == nullptr)
				continue;
			if (chunk->renderData.vertices > (void*)1){
				const int chunk_attrs[] = {3,2,4, 0};
				Mesh* mesh = new Mesh(chunk->renderData.vertices, chunk->renderData.size / CHUNK_VERTEX_SIZE, chunk_attrs);
				if (chunks->meshes[index])
					delete chunks->meshes[index];
				chunks->meshes[index] = mesh;
				delete[] chunk->renderData.vertices;
				chunk->renderData.vertices = nullptr;
			}
		}
	}
	ChunksLoader* freeLoader = getFreeLoader();
	if (freeLoader == nullptr)
		return false;

	int nearX = 0;
	int nearZ = 0;
	int minDistance = INT_MAX;
	for (int z = 1; z < d-1; z++){
		for (int x = 1; x < w-1; x++){
			int index = z * w + x;
			Chunk* chunk = chunks->chunks[index];
			if (chunk == nullptr)
				continue;
			Mesh* mesh = chunks->meshes[index];
			if (mesh != nullptr && !chunk->isModified())
				continue;
			if (!chunk->isReady() || !chunk->isLighted() || chunk->surrounding < MIN_SURROUNDING){
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
	Chunk* chunk = chunks->chunks[index];
	if (chunk == nullptr){
		return false;
	}
	Mesh* mesh = chunks->meshes[index];
	if (mesh == nullptr || chunk->isModified()){
		if (chunk->renderData.vertices != nullptr) {
			return false;
		}
		Chunk* closes[9];
		if (chunk->isEmpty()){
			chunks->meshes[index] = nullptr;
			return false;
		}

		for (int i = 0; i < 9; i++)
			closes[i] = nullptr;
		for (size_t j = 0; j < chunks->volume; j++){
			Chunk* other = chunks->chunks[j];
			if (other == nullptr)
				continue;

			int ox = other->x - chunk->x;
			int oz = other->z - chunk->z;

			if (abs(ox) > 1 || abs(oz) > 1)
				continue;

			ox += 1;
			oz += 1;
			if ((!other->isReady() || !other->isLighted()) && other != chunk)
				return false;
			closes[oz * 3 + ox] = other;
		}
		chunk->setModified(false);
		chunk->renderData.vertices = (float*)1;
		freeLoader->render(chunk, (Chunk**)closes);

		return true;
	}
	return false;
}
