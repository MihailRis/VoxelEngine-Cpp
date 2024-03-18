#include "Viewport.h"

Viewport::Viewport(uint width, uint height) 
    : width(width), height(height) {
}

uint Viewport::getWidth() const {
    return width;
}

uint Viewport::getHeight() const {
    return height;
}