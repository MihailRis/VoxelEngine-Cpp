#include "Heightmap.hpp"

#include <cmath>
#include <cstring>
#include <stdexcept>
#include <glm/glm.hpp>

static inline float smootherstep(float x) {
    return glm::smoothstep(std::floor(x), std::floor(x)+1, x);
}

static inline float sample_at(
    const float* buffer,
    uint width,
    uint x, uint y
) {
    return buffer[y*width+x];
}

static inline float sample_at(
    const float* buffer,
    uint width, uint height,
    float x, float y,
    InterpolationType interp
) {
    // std::floor is redundant here because x and y are positive values
    uint ix = static_cast<uint>(x);
    uint iy = static_cast<uint>(y);
    float val = buffer[iy*width+ix];
    if (interp == InterpolationType::NEAREST) {
        return val;
    }
    float tx = x - ix;
    float ty = y - iy;

    switch (interp) {
        case InterpolationType::LINEAR: {
            float s00 = val;
            float s10 = sample_at(buffer, width, 
                ix + 1 < width ? ix + 1 : ix, iy);
            float s01 = sample_at(buffer, width, ix, 
                iy + 1 < height ? iy + 1 : iy);
            float s11 = sample_at(buffer, width, 
                ix + 1 < width ? ix + 1 : ix, iy + 1 < height ? iy + 1 : iy);

            float a00 = s00;
            float a10 = s10 - s00;
            float a01 = s01 - s00;
            float a11 = s11 - s10 - s01 + s00;

            return a00 + a10*tx + a01*ty + a11*tx*ty;
        }
        case InterpolationType::CUBIC: {
            float b1 =
                ((tx - 1) * (tx - 2) * (tx + 1) * (ty - 1) * (ty - 2) * (ty + 1)) / 4;
            float b2 =
                (tx * (tx + 1) * (tx - 2) * (ty - 1) * (ty - 2) * (ty + 1)) / -4;
            float b3 =
                (tx * (tx + 1) * (tx - 2) * (ty - 1) * (ty - 2) * (ty + 1)) / -4;
            float b4 =
                (tx * (tx + 1) * (tx + 2) * ty * (ty + 1) * (ty - 2)) / 4;
            float b5 =
                (tx * (tx - 1) * (tx - 2) * (ty - 1) * (ty - 2) * (ty + 1)) / -12;
            float b6 =
                ((tx - 1) * (tx - 2) * (tx + 1) * ty * (ty - 1) * (ty - 2)) / -12;
            float b7 =
                (tx * (tx - 1) * (tx - 2) * ty * (ty + 1) * (ty - 2)) / 12;
            float b8 =
                (tx * (tx + 1) * (tx - 2) * ty * (ty - 1) * (ty - 2)) / 12;
            float b9 =
                (tx * (tx - 1) * (tx + 1) * (ty - 1) * (ty - 2) * (ty + 1)) / 12;
            float b10 =
                ((tx - 1) * (tx - 2) * (tx + 1) * ty * (ty - 1) * (ty + 1)) /12;
            float b11 =
                (tx * (tx - 1) * (tx - 2) * ty * (ty + 1) * (ty - 2)) / 36;
            float b12 =
                (tx * (tx - 1) * (tx + 1) * ty * (ty + 1) * (ty - 2)) / -12;
            float b13 =
                (tx * (tx + 1) * (tx - 2) * ty * (ty - 1) * (ty + 1)) / -12;
            float b14 =
                (tx * (tx - 1) * (tx + 1) * ty * (ty - 1) * (ty - 2)) / -36;
            float b15 =
                (tx * (tx - 1) * (tx - 2) * ty * (ty - 1) * (ty + 1)) / -36;
            float b16 =
                (tx * (tx - 1) * (tx + 1) * ty * (ty - 1) * (ty + 1)) / 36;


            float a1 = b1 * val;
            float a2 = b2 * sample_at(buffer, width, ix, iy + 1 < height ? iy + 1 : iy);
            float a3 = b3 * sample_at(buffer, width, ix + 1 < width ? ix + 1 : ix, iy);
            float a4 = b4 * sample_at(buffer, width,
                ix + 1 < width ? ix + 1 : ix, iy + 1 < height ? iy + 1 : iy);
            float a5 = b5 * sample_at(buffer, width, ix, iy > 1 ? iy - 1 : iy);
            float a6 = b6 * sample_at(buffer, width, ix > 1 ? ix - 1 : ix, iy);
            float a7 = b7 * sample_at(buffer, width,
                ix + 1 < width ? ix + 1 : ix, iy > 1 ? iy - 1 : iy);
            float a8 = b8 * sample_at(buffer, width,
                ix > 1 ? ix - 1 : ix, iy + 1 < height ? iy + 1 : iy);
            float a9 = b9 * sample_at(buffer, width,
                ix, iy + 2 < height ? iy + 2 : iy);
            float a10 = b10 * sample_at(buffer, width,
                ix + 2 < width ? ix + 2 : ix, iy);
            float a11 = b11 * sample_at( buffer, width,
                ix > 1 ? ix - 1 : ix, iy > 1 ? iy - 1 : iy);
            float a12 = b12 * sample_at(buffer, width,
                ix + 1 < width ? ix + 1 : ix, iy + 2 < height ? iy + 2 : iy);
            float a13 = b13 * sample_at(buffer, width,
                ix + 2 < width ? ix + 2 : ix, iy + 1 < height ? iy + 1 : iy);
            float a14 = b14 * sample_at(buffer, width,
                ix > 1 ? ix - 1 : ix, iy + 2 < height ? iy + 2 : iy);
            float a15 = b15 * sample_at(buffer, width,
                ix + 2 < width ? ix + 2 : ix, iy > 1 ? iy - 1 : iy);
            float a16 = b16 * sample_at(buffer, width,
                ix + 2 < width ? ix + 2 : ix, iy + 2 < height ? iy + 2 : iy);

            return a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 +
                a9 + a10 + a11 + a12 + a13 + a14 + a15 + a16;
        }

        default:
            throw std::runtime_error("interpolation type is not implemented");
    }
    return val;
}

void Heightmap::resize(
    uint dstwidth, uint dstheight, InterpolationType interp
) {
    if (width == dstwidth && height == dstheight) {
        return;
    }
    std::vector<float> dst;
    dst.resize(dstwidth*dstheight);

    uint index = 0;
    for (uint y = 0; y < dstheight; y++) {
        for (uint x = 0; x < dstwidth; x++, index++) {
            float sx = static_cast<float>(x) / dstwidth * width;
            float sy = static_cast<float>(y) / dstheight * height;
            dst[index] = sample_at(buffer.data(), width, height, sx, sy, interp);
        }
    }

    width = dstwidth;
    height = dstheight;
    buffer = std::move(dst);
}

void Heightmap::crop(
    uint srcx, uint srcy, uint dstwidth, uint dstheight
) {
    if (srcx + dstwidth > width || srcy + dstheight > height) {
        throw std::runtime_error(
            "crop zone is not fully inside of the source image");
    }
    if (dstwidth == width && dstheight == height) {
        return;
    }

    std::vector<float> dst;
    dst.resize(dstwidth*dstheight);

    for (uint y = 0; y < dstheight; y++) {
        std::memcpy(
            dst.data()+y*dstwidth, 
            buffer.data()+(y+srcy)*width+srcx, 
            dstwidth*sizeof(float));
    }

    width = dstwidth;
    height = dstheight;
    buffer = std::move(dst);
}

void Heightmap::clamp() {
    for (uint i = 0; i < width * height; i++) {
        buffer[i] = std::min(1.0f, std::max(0.0f, buffer[i]));
    }
}
