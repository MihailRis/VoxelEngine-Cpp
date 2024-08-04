#ifndef VOXELS_FLATWORLDGENERATOR_HPP_
#define VOXELS_FLATWORLDGENERATOR_HPP_

#include <typedefs.hpp>
#include "../voxels/WorldGenerator.hpp"

struct voxel;
class Content;

class FlatWorldGenerator : WorldGenerator {
public:
    FlatWorldGenerator(const Content* content) : WorldGenerator(content) {
    }

    void generate(voxel* voxels, int x, int z, int seed);
};

#endif  // VOXELS_FLATWORLDGENERATOR_HPP_
