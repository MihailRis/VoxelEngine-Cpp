#pragma once

#include <cmath>
#include <glm/vec2.hpp>

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

    inline glm::vec2 apply(const glm::vec2& uv) {
        float w = getWidth();
        float h = getHeight();
        return glm::vec2(u1 + uv.x * w, v1 + uv.y * h);
    }
};
