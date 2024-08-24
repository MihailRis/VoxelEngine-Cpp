#pragma once

#include <string>
#include <memory>
#include <vector>

#include "constants.hpp"
#include "typedefs.hpp"
#include "voxels/voxel.hpp"

class Content;
struct GeneratorDef;
class Heightmap;
struct Biome;

enum class ChunkPrototypeLevel {
    BIOMES, HEIGHTMAP
};

struct ChunkPrototype {
    ChunkPrototypeLevel level;

    /// @brief chunk heightmap
    std::shared_ptr<Heightmap> heightmap;
    /// @brief chunk biomes matrix
    std::vector<const Biome*> biomes;

    ChunkPrototype(
        ChunkPrototypeLevel level,
        std::shared_ptr<Heightmap> heightmap, 
        std::vector<const Biome*> biomes
    ) : level(level),
        heightmap(std::move(heightmap)), 
        biomes(std::move(biomes)) {};
};

/// @brief High-level world generation controller
class WorldGenerator {
    /// @param def generator definition
    const GeneratorDef& def;
    /// @param content world content
    const Content* content;
    /// @param seed world seed
    uint64_t seed;

    /// @brief Generate chunk prototype (see ChunkPrototype)
    /// @param x chunk position X divided by CHUNK_W
    /// @param z chunk position Y divided by CHUNK_D
    std::unique_ptr<ChunkPrototype> generatePrototype(int x, int z);

    void generateHeightmap(ChunkPrototype* prototype, int x, int z);
public:
    WorldGenerator(
        const GeneratorDef& def,
        const Content* content,
        uint64_t seed
    );
    virtual ~WorldGenerator() = default;

    /// @brief Generate complete chunk voxels
    /// @param voxels destinatiopn chunk voxels buffer
    /// @param x chunk position X divided by CHUNK_W
    /// @param z chunk position Y divided by CHUNK_D
    virtual void generate(voxel* voxels, int x, int z);

    inline static std::string DEFAULT = "core:default";
};
