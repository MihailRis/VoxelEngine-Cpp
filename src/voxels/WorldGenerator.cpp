#include "WorldGenerator.h"
#include "voxel.h"
#include "Chunk.h"

#include <math.h>
#include <glm/glm.hpp>
#include <glm/gtc/noise.hpp>

void WorldGenerator::generate(voxel* voxels, int cx, int cy, int cz){
	const float s = 0.25f;
	for (int z = 0; z < CHUNK_D; z++){
		for (int x = 0; x < CHUNK_W; x++){
			int real_x = x + cx * CHUNK_W;
			int real_z = z + cz * CHUNK_D;
			float height = glm::perlin(glm::vec3(real_x*0.0125f*s,real_z*0.0125f*s, 0.0f));
			height += glm::perlin(glm::vec3(real_x*0.025f*s,real_z*0.025f*s, 0.0f))*0.5f;
			height += glm::perlin(glm::vec3(real_x*0.05f*s,real_z*0.05f*s, 0.0f))*0.25f;
			height += glm::perlin(glm::vec3(real_x*0.1f*s,real_z*0.1f*s, 0.0f))*0.225f;
			height += glm::perlin(glm::vec3(real_x*0.2f*s,real_z*0.2f*s, 0.0f))*0.125f;
			height = height * 0.5f + 0.5f;
			height *= height;
			height *= 140.0f;
			height += 48;
			for (int y = 0; y < CHUNK_H; y++){
				int real_y = y + cy * CHUNK_H;
				int id = 0;
				if (real_y == (int)height)
					id = 2;
				else if (real_y < height)
					id = 1;
				if (real_y <= 2)
					id = 2;
				voxels[(y * CHUNK_D + z) * CHUNK_W + x].id = id;
			}
		}
	}
}
