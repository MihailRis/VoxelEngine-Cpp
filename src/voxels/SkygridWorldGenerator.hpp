#ifndef VOXELS_SKYGRIDWORLDGENERATOR_HPP_
#define VOXELS_SKYGRIDWORLDGENERATOR_HPP_

#include "../typedefs.hpp"
#include "../voxels/WorldGenerator.hpp"

struct voxel;
class Content;

class SkygridWorldGenerator : WorldGenerator {
public:

    SkygridWorldGenerator(const Content* content) : WorldGenerator(content) {}

    void generate(voxel* voxels, int x, int z, int seed);
};

#endif // VOXELS_SKYGRIDWORLDGENERATOR_HPP_
