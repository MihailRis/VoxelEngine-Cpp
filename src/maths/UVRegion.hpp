#ifndef MATHS_UVREGION_HPP_
#define MATHS_UVREGION_HPP_

#include <cmath>

struct UVRegion {
    float u1;
    float v1;
    float u2;
    float v2;

    UVRegion(float u1, float v1, float u2, float v2)
    : u1(u1), v1(v1), u2(u2), v2(v2){}

    UVRegion() : u1(0.0f), v1(0.0f), u2(1.0f), v2(1.0f){}

    inline float getWidth() const {
        return fabs(u2-u1);
    }

    inline float getHeight() const {
        return fabs(v2-v1);
    }
};

#endif // MATHS_UVREGION_HPP_
