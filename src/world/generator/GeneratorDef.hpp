#pragma once

#include <string>
#include <glm/glm.hpp>

#include "typedefs.hpp"
#include "maths/Heightmap.hpp"

class GeneratorScript {
public:
    virtual ~GeneratorScript() = default;

    virtual std::shared_ptr<Heightmap> generateHeightmap(
        const glm::ivec2& offset, const glm::ivec2& size) = 0;
};

struct GeneratorDef {
    std::string name;
    std::unique_ptr<GeneratorScript> script;

    GeneratorDef(std::string name) : name(std::move(name)) {}
    GeneratorDef(const GeneratorDef&) = delete;
};
