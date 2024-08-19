#pragma once

#include <string>
#include <glm/glm.hpp>

#include "typedefs.hpp"
#include "maths/Heightmap.hpp"

class Content;

struct BlocksLayer {
    /// @brief Layer block
    std::string block;
    
    /// @brief Layer height. -1 is resizeable layer
    int height;

    /// @brief Layer can present under the sea level (default: true) else will
    /// extend the next layer
    bool belowSeaLevel;

    struct {
        /// @brief Layer block index
        blockid_t id;
    } rt;
};

/// @brief Set of blocks layers with additional info
struct BlocksLayers {
    std::vector<BlocksLayer> layers;

    /// @brief Total height of all layers after the resizeable one
    uint lastLayersHeight;
};

struct Biome {
    std::string name;
    BlocksLayers groundLayers;
    BlocksLayers seaLayers;
};

/// @brief Generator behaviour and settings interface
class GeneratorScript {
public:
    virtual ~GeneratorScript() = default;

    /// @brief Generates a heightmap with values in range 0..1
    /// @param offset position of the heightmap top left corner in the world
    /// @param size size of the heightmap
    /// @param seed world seed
    /// @return generated heightmap of given size (can't be nullptr)
    virtual std::shared_ptr<Heightmap> generateHeightmap(
        const glm::ivec2& offset, const glm::ivec2& size, uint64_t seed) = 0;

    virtual const Biome& getBiome() const = 0;

    /// @return Number of biome parameters, that biome choosing depending on
    virtual uint getBiomeParameters() const = 0;

    /// @return Sea level (top of seaLayers)
    virtual uint getSeaLevel() const = 0;

    /// @brief Build the runtime cache
    /// @param content built content
    virtual void prepare(const Content* content) = 0;
};

/// @brief Generator information
struct GeneratorDef {
    std::string name;
    std::unique_ptr<GeneratorScript> script;

    GeneratorDef(std::string name) : name(std::move(name)) {}
    GeneratorDef(const GeneratorDef&) = delete;
};
