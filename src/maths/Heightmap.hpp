#pragma once

#include <vector>
#include <string>

#include "typedefs.hpp"

enum class InterpolationType {
    NEAREST,
    LINEAR,
    CUBIC,
};

class Heightmap {
    std::vector<float> buffer;
    uint width, height;
public:
    Heightmap(uint width, uint height)
        : width(width), height(height) {
        buffer.resize(width*height);
    }

    Heightmap(uint width, uint height, std::vector<float> buffer) 
    : width(width), height(height), buffer(std::move(buffer)) {}

    ~Heightmap() = default;

    void resize(uint width, uint height, InterpolationType interpolation);

    void crop(uint srcX, uint srcY, uint dstWidth, uint dstHeight);

    uint getWidth() const {
        return width;
    }

    uint getHeight() const {
        return height;
    }

    float get(uint x, uint y) {
        return buffer.at(y * width + x);
    }

    float getUnchecked(uint x, uint y) {
        return buffer[y * width + x];
    }

    float* getValues() {
        return buffer.data();
    }

    const float* getValues() const {
        return buffer.data();
    }
};
