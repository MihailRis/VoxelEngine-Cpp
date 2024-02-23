#include "FlatWorldGenerator.h"
#include "voxel.h"
#include "Chunk.h"

#include "../content/Content.h"
#include "../core_defs.h"

void FlatWorldGenerator::generate(voxel* voxels, int cx, int cz, int seed) {
    for (int z = 0; z < CHUNK_D; z++) {
        for (int x = 0; x < CHUNK_W; x++) {
            for (int cur_y = 0; cur_y < CHUNK_H; cur_y++){
                int id = BLOCK_AIR;
                int states = 0;

                if(cur_y == 2) {
                    id = idBazalt;
                } else if(cur_y == 6) {
                    id = idGrassBlock;
                } else if(cur_y > 2 && cur_y <= 5) {
                    id = idDirt;
                } 

                voxels[(cur_y * CHUNK_D + z) * CHUNK_W + x].id = id;
                voxels[(cur_y * CHUNK_D + z) * CHUNK_W + x].states = states;
            }
        }
    }
}