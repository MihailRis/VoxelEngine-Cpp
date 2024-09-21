#pragma once

#include <stdint.h>
#include <glm/glm.hpp>

struct StructurePlacement {
    int structure;
    glm::ivec3 position;
    uint8_t rotation;

    StructurePlacement(int structure, glm::ivec3 position, uint8_t rotation)
        : structure(structure),
          position(std::move(position)),
          rotation(rotation) {
    }
};
