#include "WorldGenerator.h"
#include "voxel.h"
#include "Chunk.h"

#include <iostream>
#include <math.h>
#include <glm/glm.hpp>
#include <glm/gtc/noise.hpp>
#define FNL_IMPL
#include "../maths/FastNoiseLite.h"
#include <time.h>

#include "../declarations.h"

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
		seed = ((unsigned short)(number*23729) xor (unsigned short)(number+16786));
		rand();
	}
	void setSeed(int number1,int number2){
		seed = (((unsigned short)(number1*23729) or (unsigned short)(number2%16786)) xor (unsigned short)(number2*number1));
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
			0.0f)*0.1f;
	height += fnlGetNoise3D(noise, real_x*0.1f*8-3465,real_z*0.1f*8+4534, 0.0f)*0.125f;
	height += fnlGetNoise3D(noise, real_x*0.4f*8+4565,real_z*0.4f*8+46456, 0.0f)*0.0625f;
	// height += fnlGetNoise3D(noise, real_x*8,real_z*8, 0.0f)*0.03f*(fnlGetNoise3D(noise, -real_x*0.0125f*8-1000,real_z*0.0125f*8+2000, 0.0f)/2+0.5f);
	height *= fnlGetNoise3D(noise, real_x*0.0125f*8+1000,real_z*0.0125f*8+1000, 0.0f)/2+0.5f;
	height += 1.0f;
	height *= 64.0f;
	return height;
}

float calc_height_faster(fnl_state *noise, int real_x, int real_z){
	float height = 0;
	height += fnlGetNoise3D(noise, real_x*0.0125f*8-125567,real_z*0.0125f*8+3546, 0.0f);
	height += fnlGetNoise3D(noise, real_x*0.025f*8+4647,real_z*0.025f*8-3436, 0.0f)*0.5f;
	height += fnlGetNoise3D(noise, real_x*0.05f*8-834176,real_z*0.05f*8+23678, 0.0f)*0.25f;
	height += fnlGetNoise3D(noise,
			real_x*0.2f*8 + fnlGetNoise3D(noise, real_x*0.1f*8-23557,real_z*0.1f*8-6568, 0.0f)*50,
			real_z*0.2f*8 + fnlGetNoise3D(noise, real_x*0.1f*8+4363,real_z*0.1f*8+4456, 0.0f)*50,
			0.0f)*0.1f;
	height += fnlGetNoise3D(noise, real_x*0.1f*8-3465,real_z*0.1f*8+4534, 0.0f)*0.125f;
	height *= fnlGetNoise3D(noise, real_x*0.0125f*8+1000,real_z*0.0125f*8+1000, 0.0f)/2+0.5f;
	height += 1.0f;
	height *= 64.0f;
	return height;
}

int generate_tree(fnl_state *noise, PseudoRandom* random, float* heights, int real_x, int real_y, int real_z, int tileSize){
	const int tileX = floor((double)real_x/(double)tileSize);
	const int tileZ = floor((double)real_z/(double)tileSize);
	random->setSeed(tileX*4325261+tileZ*12160951+tileSize*9431111);

	bool gentree = fnlGetNoise3D(noise, tileX*3.0f+633, 0.0, tileZ*3.0f) > -0.1f && (random->rand() % 10) < 7;
	if (!gentree)
		return 0;

	const int randomX = (random->rand() % (tileSize/2)) - tileSize/4;
	const int randomZ = (random->rand() % (tileSize/2)) - tileSize/4;
	int centerX = tileX * tileSize + tileSize/2 + randomX;
	int centerZ = tileZ * tileSize + tileSize/2 + randomZ;
	// int height = (int)(heights[centerX*CHUNK_W+centerZ]);
	int height = (int)calc_height_faster(noise, centerX, centerZ);
	if ((height < 57)/* || (fnlGetNoise3D(noise, real_x*0.025f,real_z*0.025f, 0.0f)*0.5f > 0.5)*/)
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
	fnl_state noise = fnlCreateState();
	noise.noise_type = FNL_NOISE_OPENSIMPLEX2;
	noise.seed = seed * 60617077 % 25896307;
	PseudoRandom randomtree;
	PseudoRandom randomgrass;

	float heights[CHUNK_VOL];

	for (int z = 0; z < CHUNK_D; z++){
		for (int x = 0; x < CHUNK_W; x++){
			int real_x = x + cx * CHUNK_W;
			int real_z = z + cz * CHUNK_D;
			float height = calc_height(&noise, real_x, real_z);
			heights[z*CHUNK_W+x] = height;
		}
	}

	for (int z = 0; z < CHUNK_D; z++){
			int real_z = z + cz * CHUNK_D;
		for (int x = 0; x < CHUNK_W; x++){
			int real_x = x + cx * CHUNK_W;
			float height = heights[z*CHUNK_W+x];

			for (int y = 0; y < CHUNK_H; y++){
				int real_y = y;
				int id = real_y < 55 ? BLOCK_WATER : BLOCK_AIR;
				int states = 0;
				if ((real_y == (int)height) && (54 < real_y)) {
					id = BLOCK_GRASS_BLOCK;
				} else if (real_y < (height - 6)){
					id = BLOCK_STONE;
				} else if (real_y < height){
					id = BLOCK_DIRT;
				} else {
					int tree = generate_tree(&noise, &randomtree, heights, real_x, real_y, real_z, 23);
					if (tree) {
						id = tree;
						states = 0x32;
					// } else if ((tree = generate_tree(&noise, &randomtree, heights, real_x, real_y, real_z, 19))){
					// 	id = tree;
					// } else if ((tree = generate_tree(&noise, &randomtree, heights, real_x, real_y, real_z, 23))){
					// 	id = tree;
					}
				}
				if ( ((height - (1.5 - 0.2 * pow(height - 54, 4))) < real_y) && (real_y < height)){
					id = BLOCK_SAND;
				}
				if (real_y <= 2)
					id = BLOCK_BEDROCK;

				randomgrass.setSeed(real_x,real_z);
				if ((id == 0) && (height > 55.5) && ((int)(height + 1) == real_y) && ((unsigned short)randomgrass.rand() > 56000)){
					id = BLOCK_GRASS;
				}
				if ((id == 0) && (height > 55.5) && ((int)(height + 1) == real_y) && ((unsigned short)randomgrass.rand() > 65000)){
					id = BLOCK_FLOWER;
				}
				if ((height > 56) && ((int)(height + 1) == real_y) && ((unsigned short)randomgrass.rand() > 65533)){
					id = BLOCK_WOOD;
					states = 0x32;
				}
				voxels[(y * CHUNK_D + z) * CHUNK_W + x].id = id;
				voxels[(y * CHUNK_D + z) * CHUNK_W + x].states = states;
			}
		}
	}
}
