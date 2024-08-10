#pragma once

#include <glm/glm.hpp>

#include "typedefs.hpp"

class Viewport {
    uint width;
    uint height;
public:
    Viewport(uint width, uint height);

    virtual uint getWidth() const;
    virtual uint getHeight() const;

    glm::ivec2 size() const {
        return glm::ivec2(width, height);
    }
};
