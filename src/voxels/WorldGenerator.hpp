#pragma once

#include <string>

#include "typedefs.hpp"

struct voxel;
class Content;

class WorldGenerator {
protected:
    blockid_t const idStone;
    blockid_t const idDirt;
    blockid_t const idGrassBlock;
    blockid_t const idSand;
    blockid_t const idWater;
    blockid_t const idWood;
    blockid_t const idLeaves;
    blockid_t const idGrass;
    blockid_t const idFlower;
    blockid_t const idBazalt;
public:
    WorldGenerator(const Content* content);
    virtual ~WorldGenerator() = default;

    virtual void generate(voxel* voxels, int x, int z, int seed) = 0;
};
