#ifndef MATHS_FASTMATHS_H_
#define MATHS_FASTMATHS_H_

namespace fastmaths {
    static unsigned int g_seed;

    inline void srand(int seed) {
        g_seed = seed;
    }

    inline int rand(void) {
        g_seed = (214013*g_seed+2531011);
        return (g_seed>>16)&0x7FFF;
    }
}

#endif // MATHS_FASTMATHS_H_
