#include "WorldGenerator.h"
#include "voxel.h"
#include "Chunk.h"
#include "Block.h"

#include <iostream>
#include <vector>
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

// TODO: do something with long conditions + move magic numbers to constants

const int SEA_LEVEL = 55;

enum class MAPS{
    SAND,
    TREE,
    CLIFF,
    HEIGHT
};
#define MAPS_LEN 4

class Map2D {
    int x, z;
    int w, d;
    float* heights[MAPS_LEN];
public:
    Map2D(int x, int z, int w, int d) : x(x), z(z), w(w), d(d) {
        for (int i = 0; i < MAPS_LEN; i++)
            heights[i] = new float[w*d];
    }
    ~Map2D() {
        for (int i = 0; i < MAPS_LEN; i++)
            delete[] heights[i];
    }

    inline float get(MAPS map, int x, int z) {
        x -= this->x;
        z -= this->z;
        if (x < 0 || z < 0 || x >= w || z >= d) {
            throw std::runtime_error("out of heightmap");
        }
        return heights[(int)map][z * w + x];
    }

    inline void set(MAPS map, int x, int z, float value) {
        x -= this->x;
        z -= this->z;
        if (x < 0 || z < 0 || x >= w || z >= d) {
            throw std::runtime_error("out of heightmap");
        }
        heights[(int)map][z * w + x] = value;
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



float calc_height(fnl_state *noise, int cur_x, int cur_z){
    float height = 0;

    height += fnlGetNoise2D(noise, cur_x*0.0125f*8-125567,cur_z*0.0125f*8+3546);
    height += fnlGetNoise2D(noise, cur_x*0.025f*8+4647,cur_z*0.025f*8-3436)*0.5f;
    height += fnlGetNoise2D(noise, cur_x*0.05f*8-834176,cur_z*0.05f*8+23678)*0.25f;
    height += fnlGetNoise2D(noise,
                            cur_x*0.2f*8 + fnlGetNoise2D(noise, cur_x*0.1f*8-23557,cur_z*0.1f*8-6568)*50,
                            cur_z*0.2f*8 + fnlGetNoise2D(noise, cur_x*0.1f*8+4363,cur_z*0.1f*8+4456)*50
                            ) * fnlGetNoise2D(noise, cur_x*0.01f-834176,cur_z*0.01f+23678) * 0.25;
    height += fnlGetNoise2D(noise, cur_x*0.1f*8-3465,cur_z*0.1f*8+4534)*0.125f;
    height *= fnlGetNoise2D(noise, cur_x*0.1f+1000,cur_z*0.1f+1000)*0.5f+0.5f;
    height += 1.0f;
    height *= 64.0f;
    return height;
}

WorldGenerator::WorldGenerator(const Content* content)
               : idStone(content->requireBlock("base:stone")->rt.id),
                 idDirt(content->requireBlock("base:dirt")->rt.id),
                 idGrassBlock(content->requireBlock("base:grass_block")->rt.id),
                 idSand(content->requireBlock("base:sand")->rt.id),
                 idWater(content->requireBlock("base:water")->rt.id),
                 idWood(content->requireBlock("base:wood")->rt.id),
                 idLeaves(content->requireBlock("base:leaves")->rt.id),
                 idGrass(content->requireBlock("base:grass")->rt.id),
                 idFlower(content->requireBlock("base:flower")->rt.id),
                 idBazalt(content->requireBlock("base:bazalt")->rt.id) {}

int generate_tree(fnl_state *noise, 
                  PseudoRandom* random, 
                  Map2D& heights, 
                //   Map2D& humidity,
                  int cur_x, 
                  int cur_y, 
                  int cur_z, 
                  int tileSize,
                  blockid_t idWood,
                  blockid_t idLeaves){
    const int tileX = floordiv(cur_x, tileSize);
    const int tileZ = floordiv(cur_z, tileSize);

    random->setSeed(tileX*4325261+tileZ*12160951+tileSize*9431111);

    int randomX = (random->rand() % (tileSize/2)) - tileSize/4;
    int randomZ = (random->rand() % (tileSize/2)) - tileSize/4;

    int centerX = tileX * tileSize + tileSize/2 + randomX;
    int centerZ = tileZ * tileSize + tileSize/2 + randomZ;

    bool gentree = (random->rand() % 10) < heights.get(MAPS::TREE, centerX, centerZ) * 13;
    if (!gentree)
        return 0;

    int height = (int)(heights.get(MAPS::HEIGHT, centerX, centerZ));
    if (height < SEA_LEVEL+1)
        return 0;
    int lx = cur_x - centerX;
    int radius = random->rand() % 4 + 2;
    int ly = cur_y - height - 3 * radius;
    int lz = cur_z - centerZ;
    if (lx == 0 && lz == 0 && cur_y - height < (3*radius + radius/2))
        return idWood;
    if (lx*lx+ly*ly/2+lz*lz < radius*radius)
        return idLeaves;
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

    for (int z = -padding; z < CHUNK_D+padding; z++){
        for (int x = -padding; x < CHUNK_W+padding; x++){
            int cur_x = x + cx * CHUNK_W;
            int cur_z = z + cz * CHUNK_D;
            float height = calc_height(&noise, cur_x, cur_z);
            float hum = fnlGetNoise2D(&noise, cur_x * 0.3 + 633, cur_z * 0.3);
            float sand = fnlGetNoise2D(&noise, cur_x * 0.1 - 633, cur_z * 0.1 + 1000);
                float cliff = pow((sand + abs(sand)) / 2, 2);
                float w = pow(fmax(-abs(height-SEA_LEVEL)+4,0)/6,2) * cliff;
                float h1 = -abs(height-SEA_LEVEL - 0.03);
                float h2 = abs(height-SEA_LEVEL + 0.04);
                float h = (h1 + h2)*100;
                height += (h * w);
            heights.set(MAPS::HEIGHT, cur_x, cur_z, height);
            heights.set(MAPS::TREE, cur_x, cur_z, hum);
            heights.set(MAPS::SAND, cur_x, cur_z, sand);
            heights.set(MAPS::CLIFF, cur_x, cur_z, cliff);
        }
    }

    for (int z = 0; z < CHUNK_D; z++){
        int cur_z = z + cz * CHUNK_D;
        for (int x = 0; x < CHUNK_W; x++){
            int cur_x = x + cx * CHUNK_W;
            float height = heights.get(MAPS::HEIGHT, cur_x, cur_z);

            for (int cur_y = 0; cur_y < CHUNK_H; cur_y++){
                // int cur_y = y;
                int id = cur_y < SEA_LEVEL ? idWater : BLOCK_AIR;
                int states = 0;
                if ((cur_y == (int)height) && (SEA_LEVEL-2 < cur_y)) {
                    id = idGrassBlock;
                } else if (cur_y < (height - 6)){
                    id = idStone;
                } else if (cur_y < height){
                    id = idDirt;
                } else {
                    int tree = generate_tree(
                        &noise, &randomtree, heights, 
                        cur_x, cur_y, cur_z, 
                        treesTile, idWood, idLeaves);
                    if (tree) {
                        id = tree;
                        states = BLOCK_DIR_UP;
                    }
                }
                float sand = fmax(heights.get(MAPS::SAND, cur_x, cur_z), heights.get(MAPS::CLIFF, cur_x, cur_z));
                if (((height -  (1.1 - 0.2 * pow(height - 54, 4)) +
                     (5*sand)) < cur_y + (height - 0.01- (int)height))
                    && (cur_y < height)){
                    id = idSand;
                }
                if (cur_y <= 2)
                    id = idBazalt;

                randomgrass.setSeed(cur_x,cur_z);
                if ((id == 0) && ((height > SEA_LEVEL+0.4) || (sand > 0.1)) && ((int)(height + 1) == cur_y) && ((unsigned short)randomgrass.rand() > 56000)){
                    id = idGrass;
                }
                if ((id == 0) && (height > SEA_LEVEL+0.4) && ((int)(height + 1) == cur_y) && ((unsigned short)randomgrass.rand() > 65000)){
                    id = idFlower;
                }
                if ((height > SEA_LEVEL+1) && ((int)(height + 1) == cur_y) && ((unsigned short)randomgrass.rand() > 65533)){
                    id = idWood;
                    states = BLOCK_DIR_UP;
                }
                voxels[(cur_y * CHUNK_D + z) * CHUNK_W + x].id = id;
                voxels[(cur_y * CHUNK_D + z) * CHUNK_W + x].states = states;
            }
        }
    }
}
