#ifndef VOXELS_DEFAULTWORLDGENERATOR_HPP_
#define VOXELS_DEFAULTWORLDGENERATOR_HPP_

#include "../typedefs.hpp"
#include "../voxels/WorldGenerator.hpp"

struct voxel;
class Content;

class DefaultWorldGenerator : WorldGenerator {
public:
    DefaultWorldGenerator(const Content* content) : WorldGenerator(content) {
    }

    void generate(voxel* voxels, int x, int z, int seed);
};

#endif  // VOXELS_DEFAULTWORLDGENERATOR_HPP_
