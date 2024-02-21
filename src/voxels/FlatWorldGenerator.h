#ifndef VOXELS_FLATWORLDGENERATOR_H_
#define VOXELS_FLATWORLDGENERATOR_H_

#include "../typedefs.h"
#include "../voxels/WorldGenerator.h"

struct voxel;
class Content;

class FlatWorldGenerator : WorldGenerator {
public:

	FlatWorldGenerator(const Content* content) : WorldGenerator(content) {}

	void generate(voxel* voxels, int x, int z, int seed) override;
};

#endif /* VOXELS_FLATWORLDGENERATOR_H_ */