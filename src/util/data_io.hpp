#pragma once

#include "typedefs.hpp"

namespace dataio {
    /* Read big-endian 16 bit signed integer from bytes */
    inline int16_t read_int16_big(const ubyte* src, size_t offset) {
        return (src[offset] << 8) | (src[offset + 1]);
    }
    /* Read big-endian 32 bit signed integer from bytes */
    inline int32_t read_int32_big(const ubyte* src, size_t offset) {
        return (src[offset] << 24) | (src[offset + 1] << 16) |
               (src[offset + 2] << 8) | (src[offset + 3]);
    }
    /* Read big-endian 64 bit signed integer from bytes */
    inline int64_t read_int64_big(const ubyte* src, size_t offset) {
        return (int64_t(src[offset]) << 56) | (int64_t(src[offset + 1]) << 48) |
               (int64_t(src[offset + 2]) << 40) |
               (int64_t(src[offset + 3]) << 32) |
               (int64_t(src[offset + 4]) << 24) |
               (int64_t(src[offset + 5]) << 16) |
               (int64_t(src[offset + 6]) << 8) | (int64_t(src[offset + 7]));
    }
    /* Write big-endian 16 bit signed integer to bytes */
    inline void write_int16_big(int16_t value, ubyte* dest, size_t offset) {
        dest[offset] = (char)(value >> 8 & 255);
        dest[offset + 1] = (char)(value >> 0 & 255);
    }
    /* Write big-endian 32 bit signed integer to bytes */
    inline void write_int32_big(int32_t value, ubyte* dest, size_t offset) {
        dest[offset] = (char)(value >> 24 & 255);
        dest[offset + 1] = (char)(value >> 16 & 255);
        dest[offset + 2] = (char)(value >> 8 & 255);
        dest[offset + 3] = (char)(value >> 0 & 255);
    }
    /* Write big-endian 64 bit signed integer to bytes */
    inline void write_int64_big(int64_t value, ubyte* dest, size_t offset) {
        dest[offset] = (char)(value >> 56 & 255);
        dest[offset + 1] = (char)(value >> 48 & 255);
        dest[offset + 2] = (char)(value >> 40 & 255);
        dest[offset + 3] = (char)(value >> 32 & 255);

        dest[offset + 4] = (char)(value >> 24 & 255);
        dest[offset + 5] = (char)(value >> 16 & 255);
        dest[offset + 6] = (char)(value >> 8 & 255);
        dest[offset + 7] = (char)(value >> 0 & 255);
    }
}
