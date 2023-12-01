#ifndef VOXELS_WORLDGENERATOR_H_
#define VOXELS_WORLDGENERATOR_H_

#include "../typedefs.h"

class voxel;
class Content;

class WorldGenerator {
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
	void generate(voxel* voxels, int x, int z, int seed);
};

#endif /* VOXELS_WORLDGENERATOR_H_ */
