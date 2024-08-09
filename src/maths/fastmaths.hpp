#ifndef MATHS_FASTMATHS_HPP_
#define MATHS_FASTMATHS_HPP_

#include "typedefs.hpp"

class FastRandom {
    uint seed;
public:
    inline void setSeed(uint seed) {
        this->seed = seed;
    }

    inline int rand() {
        seed = (214013 * seed + 2531011);
        return (seed >> 16) & 0x7FFF;
    }

    inline float randFloat() {
        return rand() / float(0x7FFF);
    }
};

#endif  // MATHS_FASTMATHS_HPP_
