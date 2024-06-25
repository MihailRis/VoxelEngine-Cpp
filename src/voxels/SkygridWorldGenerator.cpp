#include "SkygridWorldGenerator.hpp"
#include "voxel.hpp"
#include "Chunk.hpp"

#include "../content/Content.hpp"
#include "../core_defs.hpp"
#include <iostream>
#include <random>

void SkygridWorldGenerator::generate(voxel* voxels, int cx, int cz, int seed) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 5);

    int counter = 0;

    for (int z = 0; z < CHUNK_D; z++) {
        for (int x = 0; x < CHUNK_W; x++) {
            for (int cur_y = 0; cur_y < CHUNK_H; cur_y++){
                int id = BLOCK_AIR;
                blockstate state {};

                if ((x % 4 == 0) && (cur_y % 4 == 0) && (z % 4 == 0)) {
                    int randomBlockId = dis(gen);
                    switch (randomBlockId) {
                        case 1:
                            id = idGrassBlock;
                            break;
                        case 2:
                            id = idDirt;
                            break;
                        case 3:
                            id = idStone;
                            break;
                        case 4:
                            id = idSand;
                            break;
                        case 5:
                            id = idBazalt;
                            break;
                        default:
                            id = idStone;
                            break;
                    }
                }

                voxels[(cur_y * CHUNK_D + z) * CHUNK_W + x].id = id;
                voxels[(cur_y * CHUNK_D + z) * CHUNK_W + x].state = state;

                counter++;
            }
        }
    }
}
