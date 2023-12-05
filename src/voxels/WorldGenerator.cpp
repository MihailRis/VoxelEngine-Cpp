#include "WorldGenerator.h"
#include "voxel.h"
#include "Chunk.h"
#include "Block.h"

#include <iostream>
#include <time.h>
#include <stdexcept>
#include <math.h>
#include <glm/glm.hpp>
#include <glm/gtc/noise.hpp>
#define FNL_IMPL
#include "../maths/FastNoiseLite.h"

#include "../content/Content.h"
#include "../maths/voxmaths.h"
#include "../core_defs.h"

const int SEA_LEVEL = 55;

class Map2D {
	int x, z;
	int w, d;
	float* heights;
public:
	Map2D(int x, int z, int w, int d)
			: x(x), z(z), w(w), d(d) {
		heights = new float[w * d];
	}
	~Map2D() {
		delete[] heights;
	}

	inline float get(int x, int z) {
		x -= this->x;
		z -= this->z;
		if (x < 0 || z < 0 || x >= w || z >= d) {
			throw std::runtime_error("out of heightmap");
		}
		return heights[z * w + x];
	}

	inline void set(int x, int z, float value) {
		x -= this->x;
		z -= this->z;
		if (x < 0 || z < 0 || x >= w || z >= d) {
			throw std::runtime_error("out of heightmap");
		}
		heights[z * w + x] = value;
	}
};

class PseudoRandom {
	unsigned short seed;
public:
	PseudoRandom(){
		seed = (unsigned short)time(0);
	}

	int rand(){
		seed = (seed + 0x7ed5 + (seed << 6));
		seed = (seed ^ 0xc23c ^ (seed >> 9));
		seed = (seed + 0x1656 + (seed << 3));
		seed = ((seed + 0xa264) ^ (seed << 4));
		seed = (seed + 0xfd70 - (seed << 3));
		seed = (seed ^ 0xba49 ^ (seed >> 8));

		return (int)seed;
	}

	void setSeed(int number){
		seed = ((unsigned short)(number*23729) ^ (unsigned short)(number+16786));
		rand();
	}
	void setSeed(int number1,int number2){
		seed = (((unsigned short)(number1*23729) | (unsigned short)(number2%16786)) ^ (unsigned short)(number2*number1));
		rand();
	}
};

float calc_height(fnl_state *noise, int real_x, int real_z){
	float height = 0;

	height += fnlGetNoise3D(noise, real_x*0.0125f*8-125567,real_z*0.0125f*8+3546, 0.0f);
	height += fnlGetNoise3D(noise, real_x*0.025f*8+4647,real_z*0.025f*8-3436, 0.0f)*0.5f;
	height += fnlGetNoise3D(noise, real_x*0.05f*8-834176,real_z*0.05f*8+23678, 0.0f)*0.25f;
	height += fnlGetNoise3D(noise,
			real_x*0.2f*8 + fnlGetNoise3D(noise, real_x*0.1f*8-23557,real_z*0.1f*8-6568, 0.0f)*50,
			real_z*0.2f*8 + fnlGetNoise3D(noise, real_x*0.1f*8+4363,real_z*0.1f*8+4456, 0.0f)*50,
			0.0f) * fnlGetNoise3D(noise, real_x*0.01f-834176,real_z*0.01f+23678, 0.0f);
	height += fnlGetNoise3D(noise, real_x*0.1f*8-3465,real_z*0.1f*8+4534, 0.0f)*0.125f;
	//height += fnlGetNoise3D(noise, real_x*0.4f+4565,real_z*0.4f*18+46456, 0.0f)*0.0625f * 0.3f;
	height *= fnlGetNoise3D(noise, real_x*0.1f+1000,real_z*0.1f+1000, 0.0f)*0.5f+0.5f;
	height += 1.0f;
	height *= 64.0f;
	return height;
}

WorldGenerator::WorldGenerator(const Content* content)
	           : idStone(content->require("base:stone")->rt.id),
			     idDirt(content->require("base:dirt")->rt.id),
				 idGrassBlock(content->require("base:grass_block")->rt.id),
				 idSand(content->require("base:sand")->rt.id),
				 idWater(content->require("base:water")->rt.id),
				 idWood(content->require("base:wood")->rt.id),
				 idLeaves(content->require("base:leaves")->rt.id),
				 idGrass(content->require("base:grass")->rt.id),
				 idFlower(content->require("base:flower")->rt.id),
				 idBazalt(content->require("base:bazalt")->rt.id) {;
}

int generate_tree(fnl_state *noise, 
				  PseudoRandom* random, 
				  Map2D& heights, 
				  Map2D& humidity,
				  int real_x, 
				  int real_y, 
				  int real_z, 
				  int tileSize){
	const int tileX = floordiv(real_x, tileSize);
	const int tileZ = floordiv(real_z, tileSize);

	random->setSeed(tileX*4325261+tileZ*12160951+tileSize*9431111);

	int randomX = (random->rand() % (tileSize/2)) - tileSize/4;
	int randomZ = (random->rand() % (tileSize/2)) - tileSize/4;

	int centerX = tileX * tileSize + tileSize/2 + randomX;
	int centerZ = tileZ * tileSize + tileSize/2 + randomZ;

	bool gentree = (random->rand() % 10) < humidity.get(centerX, centerZ) * 13;
	if (!gentree)
		return 0;

	int height = (int)(heights.get(centerX, centerZ));
	if (height < SEA_LEVEL+1)
		return 0;
	int lx = real_x - centerX;
	int radius = random->rand() % 4 + 2;
	int ly = real_y - height - 3 * radius;
	int lz = real_z - centerZ;
	if (lx == 0 && lz == 0 && real_y - height < (3*radius + radius/2))
		return 6;
	if (lx*lx+ly*ly/2+lz*lz < radius*radius)
		return 7;
	return 0;
}

void WorldGenerator::generate(voxel* voxels, int cx, int cz, int seed){
	const int treesTile = 12;
	fnl_state noise = fnlCreateState();
	noise.noise_type = FNL_NOISE_OPENSIMPLEX2;
	noise.seed = seed * 60617077 % 25896307;
	PseudoRandom randomtree;
	PseudoRandom randomgrass;

	int padding = 8;
	Map2D heights(cx * CHUNK_W - padding, 
					  cz * CHUNK_D - padding, 
					  CHUNK_W + padding * 2, 
					  CHUNK_D + padding * 2);

	
	// Influences to trees and sand generation
	Map2D humidity(cx * CHUNK_W - padding, 
					cz * CHUNK_D - padding, 
					CHUNK_W + padding * 2, 
					CHUNK_D + padding * 2);

	for (int z = -padding; z < CHUNK_D+padding; z++){
		for (int x = -padding; x < CHUNK_W+padding; x++){
			int real_x = x + cx * CHUNK_W;
			int real_z = z + cz * CHUNK_D;
			float height = calc_height(&noise, real_x, real_z);
			float hum = fnlGetNoise3D(&noise, real_x * 0.3 + 633, 0.0, real_z * 0.3);
			if (height >= SEA_LEVEL) {
				height = ((height - SEA_LEVEL) * 0.1) - 0.0;
				height = powf(height, (1.0+hum - fmax(0.0, height) * 0.2));
				height = height * 10 + SEA_LEVEL;
			} else {
				height *= 1.0f + (height-SEA_LEVEL) * 0.05f * hum;
			}
			heights.set(real_x, real_z, height);
			humidity.set(real_x, real_z, hum);
		
		}
	}

	for (int z = 0; z < CHUNK_D; z++){
		int real_z = z + cz * CHUNK_D;
		for (int x = 0; x < CHUNK_W; x++){
			int real_x = x + cx * CHUNK_W;
			float height = heights.get(real_x, real_z);

			for (int y = 0; y < CHUNK_H; y++){
				int real_y = y;
				int id = real_y < SEA_LEVEL ? idWater : BLOCK_AIR;
				int states = 0;
				if ((real_y == (int)height) && (SEA_LEVEL-2 < real_y)) {
					id = idGrassBlock;
				} else if (real_y < (height - 6)){
					id = idStone;
				} else if (real_y < height){
					id = idDirt;
				} else {
					int tree = generate_tree(&noise, &randomtree, heights, humidity, real_x, real_y, real_z, treesTile);
					if (tree) {
						id = tree;
						states = BLOCK_DIR_UP;
					}
				}
				if (((height - (1.5 - 0.2 * pow(height - 54, 4))) < real_y) && (real_y < height) && humidity.get(real_x, real_z) < 0.1){
					id = idSand;
				}
				if (real_y <= 2)
					id = idBazalt;

				randomgrass.setSeed(real_x,real_z);
				if ((id == 0) && (height > SEA_LEVEL+0.5) && ((int)(height + 1) == real_y) && ((unsigned short)randomgrass.rand() > 56000)){
					id = idGrass;
				}
				if ((id == 0) && (height > SEA_LEVEL+0.5) && ((int)(height + 1) == real_y) && ((unsigned short)randomgrass.rand() > 65000)){
					id = idFlower;
				}
				if ((height > SEA_LEVEL+1) && ((int)(height + 1) == real_y) && ((unsigned short)randomgrass.rand() > 65533)){
					id = idWood;
					states = BLOCK_DIR_UP;
				}
				voxels[(y * CHUNK_D + z) * CHUNK_W + x].id = id;
				voxels[(y * CHUNK_D + z) * CHUNK_W + x].states = states;
			}
		}
	}
}
