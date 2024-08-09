///    C++ LMPacker port
///        https://github.com/MihailRis/LMPacker
#pragma once

#include <stdint.h>
#include <stdlib.h>

#include <memory>
#include <vector>

struct rectangle {
    unsigned int idx;
    int x;
    int y;
    int width;
    int height;
    int extX = 0;
    int extY = 0;

    rectangle(unsigned int idx, int x, int y, int width, int height)
        : idx(idx), x(x), y(y), width(width), height(height) {
    }
};

class LMPacker {
public:
    using matrix_row = std::unique_ptr<rectangle*[]>;
    using matrix_ptr = std::unique_ptr<matrix_row[]>;
private:
    std::vector<rectangle> rects;
    std::vector<rectangle*> placed;
    uint32_t width = 0;
    uint32_t height = 0;
    matrix_ptr matrix = nullptr;
    uint32_t mbit = 0;

    void cleanup();
    bool place(rectangle* rect, uint32_t vstep);
public:
    LMPacker(const uint32_t sizes[], size_t length);
    virtual ~LMPacker();

    bool buildCompact(uint32_t width, uint32_t height, uint16_t extension) {
        return build(width, height, extension, 0, 1);
    }
    bool buildFast(uint32_t width, uint32_t height, uint16_t extension) {
        return build(width, height, extension, 1, 2);
    }
    bool build(
        uint32_t width,
        uint32_t height,
        uint16_t extension,
        uint32_t mbit,
        uint32_t vstep
    );

    std::vector<rectangle> getResult() {
        return rects;
    }
};
