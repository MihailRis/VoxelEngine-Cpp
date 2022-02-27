#include "WorldGenerator.h"
#include "voxel.h"
#include "Chunk.h"

#include <math.h>
#include <glm/glm.hpp>
#include <glm/gtc/noise.hpp>
#define FNL_IMPL
#include "../maths/FastNoiseLite.h"
#include <time.h>

class PseudoRandom {
	unsigned seed;
public:
	PseudoRandom(){
		seed = (unsigned)time(0);
	}

	int rand(){
		seed = (8253729 * seed + 2396403);
	    return seed % 32768;
	}

	void setSeed(int number){
		seed = (unsigned)number+8253729;
		rand();
	}
};

float calc_height(fnl_state *noise, int real_x, int real_z){
	const float s = 0.2f;
	float height = fnlGetNoise3D(noise, real_x*0.0125f*s*32,real_z*0.0125f*s*32, 0.0f);
	height += fnlGetNoise3D(noise, real_x*0.025f*s*32,real_z*0.025f*s*32, 0.0f)*0.5f;
	height += fnlGetNoise3D(noise, real_x*0.05f*s*32,real_z*0.05f*s*32, 0.0f)*0.25f;
	height += fnlGetNoise3D(noise, real_x*0.1f*s*32,real_z*0.1f*s*32, 0.0f)*0.225f;
	height += fnlGetNoise3D(noise, real_x*0.2f*s*32,real_z*0.2f*s*32, 0.0f)*0.125f;
	height += fnlGetNoise3D(noise, real_x*0.4f*s*32,real_z*0.4f*s*32, 0.0f)*0.125f*0.5F;
	height = height * 0.5f + 0.5f;
	height *= height;
	height *= (140.0f)*0.12f/s;
	height += (42)*0.12f/s;
	return height;
}

float calc_height_faster(fnl_state *noise, int real_x, int real_z){
	const float s = 0.2f;
	float height = fnlGetNoise3D(noise, real_x*0.0125f*s*32,real_z*0.0125f*s*32, 0.0f);
	height += fnlGetNoise3D(noise, real_x*0.025f*s*32,real_z*0.025f*s*32, 0.0f)*0.5f;
	height += fnlGetNoise3D(noise, real_x*0.05f*s*32,real_z*0.05f*s*32, 0.0f)*0.25f;
	height += fnlGetNoise3D(noise, real_x*0.1f*s*32,real_z*0.1f*s*32, 0.0f)*0.225f;
	height += fnlGetNoise3D(noise, real_x*0.2f*s*32,real_z*0.2f*s*32, 0.0f)*0.125f;
	//height += fnlGetNoise3D(noise, real_x*0.4f*s*32,real_z*0.4f*s*32, 0.0f)*0.125f*0.5F;
	height = height * 0.5f + 0.5f;
	height *= height;
	height *= (140.0f)*0.12f/s;
	height += (42)*0.12f/s;
	return height;
}
#include <iostream>
int generate_tree(fnl_state *noise, PseudoRandom* random, const float* heights, int real_x, int real_y, int real_z, int tileSize){
	const int tileX = floor((double)real_x/(double)tileSize);
	const int tileY = floor((double)real_z/(double)tileSize);
	random->setSeed(tileX*4325261+tileY*12160951+tileSize*9431111);

	bool gentree = fnlGetNoise3D(noise, tileX*3.0f+633, 0.0, tileY*3.0f) > -0.1f && (random->rand() % 10) < 7;
	if (!gentree)
		return 0;

	const int randomX = (random->rand() % (tileSize/2)) - tileSize/4;
	const int randomZ = (random->rand() % (tileSize/2)) - tileSize/4;
	int centerX = tileX * tileSize + tileSize/2 + randomX;
	int centerY = tileY * tileSize + tileSize/2 + randomZ;
	int height = (int)calc_height_faster(noise, centerX, centerY);
	if (height < 55)
		return 0;
	int lx = real_x - centerX;
	int radius = random->rand() % 4 + 3;
	int ly = real_y - height - 3 * radius;
	int lz = real_z - centerY;
	if (lx == 0 && lz == 0 && real_y - height < 4*radius)
		return 6;
	if (lx*lx+ly*ly/2+lz*lz < radius*radius)
		return 7;
	return 0;
}

void WorldGenerator::generate(voxel* voxels, int cx, int cy, int cz){
	fnl_state noise = fnlCreateState();
	noise.noise_type = FNL_NOISE_OPENSIMPLEX2;

	PseudoRandom random;

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
		for (int x = 0; x < CHUNK_W; x++){
			int real_x = x + cx * CHUNK_W;
			int real_z = z + cz * CHUNK_D;
			float height = heights[z*CHUNK_W+x];

			for (int y = 0; y < CHUNK_H; y++){
				int real_y = y + cy * CHUNK_H;
				int id = real_y < 55 ? 9 : 0;
				if (real_y == (int)height)
					id = 2;
				else if (real_y < height){
					if (real_y < height-6)
						id = 8;
					else
						id = 1;
				} else {
					int tree = generate_tree(&noise, &random, heights, real_x, real_y, real_z, 16);
					if (tree)
						id = tree;
					else if ((tree = generate_tree(&noise, &random, heights, real_x, real_y, real_z, 19))){
						id = tree;
					}else if ((tree = generate_tree(&noise, &random, heights, real_x, real_y, real_z, 23))){
						id = tree;
					}
				}

				if (real_y <= 2)
					id = 2;
				voxels[(y * CHUNK_D + z) * CHUNK_W + x].id = id;
			}
		}
	}
}
