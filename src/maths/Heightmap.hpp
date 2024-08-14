#pragma once

#include <vector>
#include <string>

#include "typedefs.hpp"

class Heightmap {
    std::vector<float> buffer;
    uint width, height;
public:
    Heightmap(uint width, uint height)
        : width(width), height(height) {
        buffer.resize(width*height);
    }

    ~Heightmap() = default;

    uint getWidth() const {
        return width;
    }

    uint getHeight() const {
        return height;
    }

    float* getValues() {
        return buffer.data();
    }

    const float* getValues() const {
        return buffer.data();
    }
};
