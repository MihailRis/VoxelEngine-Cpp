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

class GeneratorScript {
public:
    virtual ~GeneratorScript() = default;

    virtual std::shared_ptr<Heightmap> generateHeightmap(
        const glm::ivec2& offset, const glm::ivec2& size) = 0;

    virtual const std::vector<BlocksLayer>& getLayers() const = 0;

    /// @brief Total height of all layers after resizeable one
    virtual uint getLastLayersHeight() const = 0;

    virtual uint getSeaLevel() const = 0;

    virtual void prepare(const Content* content) = 0;
};

struct GeneratorDef {
    std::string name;
    std::unique_ptr<GeneratorScript> script;

    GeneratorDef(std::string name) : name(std::move(name)) {}
    GeneratorDef(const GeneratorDef&) = delete;
};
