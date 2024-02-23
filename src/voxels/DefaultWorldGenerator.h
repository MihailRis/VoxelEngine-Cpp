#ifndef VOXELS_WORLDGENERATOR_H_
#define VOXELS_WORLDGENERATOR_H_

#include "../typedefs.h"
#include "../voxels/WorldGenerator.h"
#include <string>

struct voxel;
class Content;

class DefaultWorldGenerator : WorldGenerator {
public:
	DefaultWorldGenerator(const Content* content) : WorldGenerator(content) {}

	void generate(voxel* voxels, int x, int z, int seed) override;
};

#endif /* VOXELS_WORLDGENERATOR_H_ */