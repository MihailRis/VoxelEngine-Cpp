#include "Lighting.h"
#include "LightSolver.h"
#include "Lightmap.h"
#include "../voxels/Chunks.h"
#include "../voxels/Chunk.h"
#include "../voxels/voxel.h"
#include "../voxels/Block.h"

#include <iostream>

Lighting::Lighting(Chunks* chunks){
	this->chunks = chunks;
	solverR = new LightSolver(chunks, 0);
	solverG = new LightSolver(chunks, 1);
	solverB = new LightSolver(chunks, 2);
	solverS = new LightSolver(chunks, 3);
}

Lighting::~Lighting(){
	delete solverR;
	delete solverG;
	delete solverB;
	delete solverS;
}

void Lighting::clear(){
	for (unsigned int index = 0; index < chunks->volume; index++){
		Chunk* chunk = chunks->chunks[index];
		if (chunk == nullptr)
			continue;
		Lightmap* lightmap = chunk->lightmap;
		for (int i = 0; i < CHUNK_VOL; i++){
			lightmap->map[i] = 0;
		}
	}
}

void Lighting::prebuildSkyLight(int cx, int cz){
	Chunk* chunk = chunks->getChunk(cx, cz);
	int highestPoint = 0;
	for (int z = 0; z < CHUNK_D; z++){
		for (int x = 0; x < CHUNK_W; x++){
			for (int y = CHUNK_H-1;;y--){
				if (y < 0)
					break;
				voxel* vox = &(chunk->voxels[(y * CHUNK_D + z) * CHUNK_W + x]);
				Block* block = Block::blocks[vox->id];
				if (!block->skyLightPassing) {
					if (highestPoint < y)
						highestPoint = y;
					break;
				}
				chunk->lightmap->setS(x,y,z, 15);
			}
		}
	}
	if (highestPoint < CHUNK_H-1)
		highestPoint++;
	chunk->lightmap->highestPoint = highestPoint;
}

void Lighting::buildSkyLight(int cx, int cz){
	Chunk* chunk = chunks->getChunk(cx, cz);
	for (int z = 0; z < CHUNK_D; z++){
		for (int x = 0; x < CHUNK_W; x++){
			for (int y = chunk->lightmap->highestPoint; y >= 0; y--){
				int gx = x + cx * CHUNK_W;
				int gz = z + cz * CHUNK_D;
				while (y > 0 && !Block::blocks[chunk->voxels[(y * CHUNK_D + z) * CHUNK_W + x].id]->lightPassing) {
					y--;
				}
				if (chunk->lightmap->getS(x, y, z) != 15) {
					solverS->add(gx,y+1,gz);
					for (; y >= 0; y--){
						solverS->add(gx+1,y,gz);
						solverS->add(gx-1,y,gz);
						solverS->add(gx,y,gz+1);
						solverS->add(gx,y,gz-1);
					}
				}
			}
		}
	}
	solverS->solve();
}

void Lighting::onChunkLoaded(int cx, int cz){
	const Chunk* chunk = chunks->getChunk(cx, cz);

	for (unsigned int y = 0; y < CHUNK_H; y++){
		for (unsigned int z = 0; z < CHUNK_D; z++){
			for (unsigned int x = 0; x < CHUNK_W; x++){
				voxel vox = chunk->voxels[(y * CHUNK_D + z) * CHUNK_W + x];
				Block* block = Block::blocks[vox.id];
				int gx = x + cx * CHUNK_W;
				int gz = z + cz * CHUNK_D;
				if (block->emission[0] || block->emission[1] || block->emission[2]){
					solverR->add(gx,y,gz,block->emission[0]);
					solverG->add(gx,y,gz,block->emission[1]);
					solverB->add(gx,y,gz,block->emission[2]);
				}
			}
		}
	}

	for (int y = -1; y <= CHUNK_H; y++){
		for (int z = -1; z <= CHUNK_D; z++){
			for (int x = -1; x <= CHUNK_W; x++){
				if (!(x == -1 || x == CHUNK_W || z == -1 || z == CHUNK_D))
					continue;
				int gx = x + cx * CHUNK_W;
				int gz = z + cz * CHUNK_D;
				if (chunks->getLight(x,y,z)){
					solverR->add(gx,y,gz);
					solverG->add(gx,y,gz);
					solverB->add(gx,y,gz);
					solverS->add(gx,y,gz);
				}
			}
		}
	}
	solverR->solve();
	solverG->solve();
	solverB->solve();
	solverS->solve();
}

void Lighting::onBlockSet(int x, int y, int z, int id){
	Block* block = Block::blocks[id];
	if (id == 0){
		solverR->remove(x,y,z);
		solverG->remove(x,y,z);
		solverB->remove(x,y,z);
		solverR->solve();
		solverG->solve();
		solverB->solve();
		if (chunks->getLight(x,y+1,z, 3) == 0xF){
			for (int i = y; i >= 0; i--){
				voxel* vox = chunks->get(x,i,z);
				if ((vox == nullptr || vox->id != 0) && Block::blocks[id]->skyLightPassing)
					break;
				solverS->add(x,i,z, 0xF);
			}
		}
		solverR->add(x,y+1,z); solverG->add(x,y+1,z); solverB->add(x,y+1,z); solverS->add(x,y+1,z);
		solverR->add(x,y-1,z); solverG->add(x,y-1,z); solverB->add(x,y-1,z); solverS->add(x,y-1,z);
		solverR->add(x+1,y,z); solverG->add(x+1,y,z); solverB->add(x+1,y,z); solverS->add(x+1,y,z);
		solverR->add(x-1,y,z); solverG->add(x-1,y,z); solverB->add(x-1,y,z); solverS->add(x-1,y,z);
		solverR->add(x,y,z+1); solverG->add(x,y,z+1); solverB->add(x,y,z+1); solverS->add(x,y,z+1);
		solverR->add(x,y,z-1); solverG->add(x,y,z-1); solverB->add(x,y,z-1); solverS->add(x,y,z-1);
		solverR->solve();
		solverG->solve();
		solverB->solve();
		solverS->solve();
	} else {
		solverR->remove(x,y,z);
		solverG->remove(x,y,z);
		solverB->remove(x,y,z);
		if (!block->skyLightPassing){
			solverS->remove(x,y,z);
			for (int i = y-1; i >= 0; i--){
				solverS->remove(x,i,z);
				if (i == 0 || chunks->get(x,i-1,z)->id != 0){
					break;
				}
			}
			solverS->solve();
		}
		solverR->solve();
		solverG->solve();
		solverB->solve();

		if (block->emission[0] || block->emission[1] || block->emission[2]){
			solverR->add(x,y,z,block->emission[0]);
			solverG->add(x,y,z,block->emission[1]);
			solverB->add(x,y,z,block->emission[2]);
			solverR->solve();
			solverG->solve();
			solverB->solve();
		}
	}
}
