#pragma once

#include <cmath>

struct UVRegion {
    float u1;
    float v1;
    float u2;
    float v2;

    UVRegion(float u1, float v1, float u2, float v2)
        : u1(u1), v1(v1), u2(u2), v2(v2) {
    }

    UVRegion() : u1(0.0f), v1(0.0f), u2(1.0f), v2(1.0f) {
    }

    inline float getWidth() const {
        return fabs(u2 - u1);
    }

    inline float getHeight() const {
        return fabs(v2 - v1);
    }

    void autoSub(float w, float h, float x, float y) {
        x *= 1.0f - w;
        y *= 1.0f - h;
        float uvw = getWidth();
        float uvh = getHeight();
        u1 = u1 + uvw * x;
        v1 = v1 + uvh * y;
        u2 = u1 + uvw * w;
        v2 = v1 + uvh * h;
    }
};
