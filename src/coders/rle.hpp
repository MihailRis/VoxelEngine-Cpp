#ifndef CODERS_RLE_HPP_
#define CODERS_RLE_HPP_

#include <typedefs.hpp>

namespace rle {
    size_t encode(const ubyte* src, size_t length, ubyte* dst);
    size_t decode(const ubyte* src, size_t length, ubyte* dst);
}

namespace extrle {
    constexpr uint max_sequence = 0x7FFF;
    size_t encode(const ubyte* src, size_t length, ubyte* dst);
    size_t decode(const ubyte* src, size_t length, ubyte* dst);
}

#endif  // CODERS_RLE_HPP_
