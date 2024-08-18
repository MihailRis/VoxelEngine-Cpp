#pragma once

#include <string>
#include <glm/glm.hpp>

#include "typedefs.hpp"
#include "maths/Heightmap.hpp"

class Content;

struct BlocksLayer {
    std::string block;
    int height;
    bool below_sea_level;

    struct {
        blockid_t id;
    } rt;
};

struct BlocksLayers {
    std::vector<BlocksLayer> layers;
    uint lastLayersHeight;
};

class GeneratorScript {
public:
    virtual ~GeneratorScript() = default;

    virtual std::shared_ptr<Heightmap> generateHeightmap(
        const glm::ivec2& offset, const glm::ivec2& size, uint64_t seed) = 0;

    virtual const BlocksLayers& getGroundLayers() const = 0;
    virtual const BlocksLayers& getSeaLayers() const = 0;

    virtual uint getSeaLevel() const = 0;

    virtual void prepare(const Content* content) = 0;
};

struct GeneratorDef {
    std::string name;
    std::unique_ptr<GeneratorScript> script;

    GeneratorDef(std::string name) : name(std::move(name)) {}
    GeneratorDef(const GeneratorDef&) = delete;
};
