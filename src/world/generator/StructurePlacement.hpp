#pragma once

#include <glm/glm.hpp>

struct StructurePlacement {
    int structure;

    glm::ivec3 position;

    StructurePlacement(int structure, glm::ivec3 position)
        : structure(structure), position(std::move(position)) {}
};
