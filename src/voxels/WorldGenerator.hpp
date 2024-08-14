#pragma once

#include <string>

#include "typedefs.hpp"

struct voxel;
class Content;
class GeneratorDef;

class WorldGenerator {
    const GeneratorDef& def;
    const Content* content;
public:
    WorldGenerator(
        const GeneratorDef& def,
        const Content* content
    );
    virtual ~WorldGenerator() = default;

    virtual void generate(voxel* voxels, int x, int z, int seed);

    inline static std::string DEFAULT = "core:default";
};
