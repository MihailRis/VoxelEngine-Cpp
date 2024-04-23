#ifndef GRAPHICS_CORE_VIEWPORT_HPP_
#define GRAPHICS_CORE_VIEWPORT_HPP_

#include <glm/glm.hpp>

#include "../../typedefs.h"

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

#endif // GRAPHICS_VIEWPORT_HPP_
