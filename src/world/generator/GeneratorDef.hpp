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

struct BiomeParameter {
    /// @brief Central parameter value for the biome
    float value;
    /// @brief Parameter score multiplier 
    /// (the higher the weight, the greater the chance of biome choosing)
    float weight;
};

struct PlantEntry {
    /// @brief Plant block id
    std::string block;
    /// @brief Plant weight
    float weight;

    struct {
        blockid_t id;
    } rt;
};

struct BiomePlants {
    static inline float MIN_CHANCE = 0.000001f;

    /// @brief Plant entries sorted by weight descending.
    std::vector<PlantEntry> plants;
    /// @brief Sum of weight values
    float weightsSum;
    /// @brief Plant generation chance
    float chance;

    /// @brief Choose plant based on weight
    /// @param rand some random value in range [0, 1)
    /// @return index of chosen plant block
    inline blockid_t choose(float rand) const {
        if (plants.empty() || rand > chance || chance < MIN_CHANCE) {
            return 0;
        }
        rand = rand / chance;
        rand *= weightsSum;
        for (const auto& plant : plants) {
            rand -= plant.weight;
            if (rand <= 0.0f) {
                return plant.rt.id;
            }
        }
        return plants[plants.size()-1].rt.id;
    }
};

struct Biome {
    std::string name;
    std::vector<BiomeParameter> parameters;
    BiomePlants plants;
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

    virtual std::vector<std::shared_ptr<Heightmap>> generateParameterMaps(
        const glm::ivec2& offset, const glm::ivec2& size, uint64_t seed) = 0;

    virtual const std::vector<Biome>& getBiomes() const = 0;

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
    /// @brief Generator full name - packid:name
    std::string name;
    std::unique_ptr<GeneratorScript> script;

    GeneratorDef(std::string name) : name(std::move(name)) {}
    GeneratorDef(const GeneratorDef&) = delete;
};
