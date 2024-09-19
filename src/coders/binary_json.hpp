#pragma once

#include <memory>
#include <vector>

#include "data/dv.hpp"

#include "typedefs.hpp"

namespace json {
    inline constexpr int BJSON_END = 0x0;
    inline constexpr int BJSON_TYPE_DOCUMENT = 0x1;
    inline constexpr int BJSON_TYPE_LIST = 0x2;
    inline constexpr int BJSON_TYPE_BYTE = 0x3;
    inline constexpr int BJSON_TYPE_INT16 = 0x4;
    inline constexpr int BJSON_TYPE_INT32 = 0x5;
    inline constexpr int BJSON_TYPE_INT64 = 0x6;
    inline constexpr int BJSON_TYPE_NUMBER = 0x7;
    inline constexpr int BJSON_TYPE_STRING = 0x8;
    inline constexpr int BJSON_TYPE_BYTES = 0x9;
    inline constexpr int BJSON_TYPE_FALSE = 0xA;
    inline constexpr int BJSON_TYPE_TRUE = 0xB;
    inline constexpr int BJSON_TYPE_NULL = 0xC;
    inline constexpr int BJSON_TYPE_CDOCUMENT = 0x1F;

    std::vector<ubyte> to_binary(const dv::value& obj, bool compress = false);
    
    dv::value from_binary(const ubyte* src, size_t size);
}
