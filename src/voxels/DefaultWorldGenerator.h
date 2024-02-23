#ifndef VOXELS_DEFAULTWORLDGENERATOR_H_
#define VOXELS_DEFAULTWORLDGENERATOR_H_

#include "../typedefs.h"
#include "../voxels/WorldGenerator.h"

struct voxel;
class Content;

class DefaultWorldGenerator : WorldGenerator {
public:

	DefaultWorldGenerator(const Content* content) : WorldGenerator(content) {}

	void generate(voxel* voxels, int x, int z, int seed);
};

#endif /* VOXELS_DEFAULTWORLDGENERATOR_H_ */