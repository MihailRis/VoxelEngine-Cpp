#pragma once

#include "typedefs.hpp"

namespace dataio {
    /// @brief Swap byte-order for value of type T
    /// @tparam T value type
    /// @param value source integer
    /// @return swapped integer
    template <typename T>
    T swap(T value) {
        union{
            T value;
            ubyte bytes[sizeof(T)];
        } source, dest;

        source.value = value;

        for (size_t i = 0; i < sizeof(T); i++) {
            dest.bytes[i] = source.bytes[sizeof(T) - i - 1];
        }
        return dest.value;
    }

    inline bool is_big_endian() {
        uint16_t num = 1;
        auto bytes = reinterpret_cast<uint8_t*>(&num);
        return bytes[1] == 1;
    }

    /// @brief Convert big-endian to hardware byte-order
    /// @tparam T value type
    /// @param value source integer
    /// @return integer with hardware byte-order
    template <typename T>
    inline T be2h(T value){
        if (is_big_endian()) {
            return value;
        } else {
            return swap(value);
        }
    }

    /// @brief Convert hardware byte-order to big-endian 
    /// @tparam T value type
    /// @param value source integer
    /// @return big-endian integer
    template <typename T>
    T h2be(T value){
        return be2h(value);
    }

    /// @brief Convert little-endian to hardware byte-order
    /// @tparam T value type
    /// @param value source integer
    /// @return integer with hardware byte-order
    template <typename T>
    T le2h(T value){
        if (is_big_endian()) {
            return swap(value);
        } else {
            return value;
        }
    }

    /// @brief Convert hardware byte-order to little-endian 
    /// @tparam T value type
    /// @param value source integer
    /// @return little-endian integer
    template <typename T>
    T h2le(T value){
        return le2h(value);
    }

    /// @brief Read big-endian 16 bit signed integer from bytes
    inline int16_t read_int16_big(const ubyte* src, size_t offset) {
        return (src[offset] << 8) | (src[offset + 1]);
    }
    /// @brief Read big-endian 32 bit signed integer from bytes
    inline int32_t read_int32_big(const ubyte* src, size_t offset) {
        return (src[offset] << 24) | (src[offset + 1] << 16) |
               (src[offset + 2] << 8) | (src[offset + 3]);
    }
    /// @brief Read big-endian 64 bit signed integer from bytes
    inline int64_t read_int64_big(const ubyte* src, size_t offset) {
        return (int64_t(src[offset]) << 56) | (int64_t(src[offset + 1]) << 48) |
               (int64_t(src[offset + 2]) << 40) |
               (int64_t(src[offset + 3]) << 32) |
               (int64_t(src[offset + 4]) << 24) |
               (int64_t(src[offset + 5]) << 16) |
               (int64_t(src[offset + 6]) << 8) | (int64_t(src[offset + 7]));
    }
    /// @brief Write big-endian 16 bit signed integer to bytes
    inline void write_int16_big(int16_t value, ubyte* dest, size_t offset) {
        dest[offset] = (char)(value >> 8 & 255);
        dest[offset + 1] = (char)(value >> 0 & 255);
    }
    /// @brief Write big-endian 32 bit signed integer to bytes
    inline void write_int32_big(int32_t value, ubyte* dest, size_t offset) {
        dest[offset] = (char)(value >> 24 & 255);
        dest[offset + 1] = (char)(value >> 16 & 255);
        dest[offset + 2] = (char)(value >> 8 & 255);
        dest[offset + 3] = (char)(value >> 0 & 255);
    }
    /// @brief Write big-endian 64 bit signed integer to bytes
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
