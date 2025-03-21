#include "Viewport.hpp"

Viewport::Viewport(uint width, uint height) 
    : width(width), height(height) {
}

Viewport::Viewport(const glm::ivec2& size) : width(size.x), height(size.y) {
}

uint Viewport::getWidth() const {
    return width;
}

uint Viewport::getHeight() const {
    return height;
}
