#ifndef GRAPHICS_CORE_VIEWPORT_HPP_
#define GRAPHICS_CORE_VIEWPORT_HPP_

#include <glm/glm.hpp>

#include "../../typedefs.hpp"

class Viewport {
    uint width;
    uint height;
public:
    Viewport(uint width, uint height);

    [[nodiscard]] virtual uint getWidth() const;
    [[nodiscard]] virtual uint getHeight() const;

    [[nodiscard]] glm::ivec2 size() const {
        return {width, height};
    }
};

#endif // GRAPHICS_VIEWPORT_HPP_
