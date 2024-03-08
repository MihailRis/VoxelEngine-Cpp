#ifndef GRAPHICS_VIEWPORT_H_
#define GRAPHICS_VIEWPORT_H_

#include <glm/glm.hpp>

#include "../typedefs.h"

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

#endif // GRAPHICS_VIEWPORT_H_