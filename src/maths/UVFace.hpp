#pragma once

#include <array>
#include <glm/glm.hpp>

#include "UVRegion.hpp"

struct UVFace {
    std::array<glm::vec2, 4> points;

    UVFace(const UVRegion& region) {
        points[0] = {region.u1, region.v1};
        points[1] = {region.u2, region.v1};
        points[2] = {region.u2, region.v2};
        points[3] = {region.u1, region.v2};
    }

    template<int n>
    inline void rotate() {
        int times = n % 4;
        if (times < 0) {
            times += 4;
        }
        std::array<glm::vec2, 4> temp = points;
        points[0] = temp[times];
        points[1] = temp[(times + 1) % 4];
        points[2] = temp[(times + 2) % 4];
        points[3] = temp[(times + 3) % 4];
    }
};
