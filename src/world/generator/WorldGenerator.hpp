#pragma once

#include <string>

#include "typedefs.hpp"

struct voxel;
class Content;
struct GeneratorDef;

/// @brief High-level world generation controller
class WorldGenerator {
    const GeneratorDef& def;
    const Content* content;
public:
    WorldGenerator(
        const GeneratorDef& def,
        const Content* content
    );
    virtual ~WorldGenerator() = default;

    /// @brief Generate complete chunk voxels
    /// @param voxels destinatiopn chunk voxels buffer
    /// @param x chunk position X divided by CHUNK_W
    /// @param z chunk position Y divided by CHUNK_D
    /// @param seed world seed
    virtual void generate(voxel* voxels, int x, int z, uint64_t seed);

    inline static std::string DEFAULT = "core:default";
};
