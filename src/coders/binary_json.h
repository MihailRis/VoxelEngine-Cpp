#ifndef CODERS_BINARY_JSON_H_
#define CODERS_BINARY_JSON_H_

#include <vector>
#include <memory>
#include "../data/dynamic.h"

namespace json {
    const int BJSON_END = 0x0;
    const int BJSON_TYPE_DOCUMENT = 0x1;
    const int BJSON_TYPE_LIST = 0x2;
    const int BJSON_TYPE_BYTE = 0x3;
    const int BJSON_TYPE_INT16 = 0x4;
    const int BJSON_TYPE_INT32 = 0x5;
    const int BJSON_TYPE_INT64 = 0x6;
    const int BJSON_TYPE_NUMBER = 0x7;
    const int BJSON_TYPE_STRING = 0x8;
    const int BJSON_TYPE_BYTES = 0x9;
    const int BJSON_TYPE_FALSE = 0xA;
    const int BJSON_TYPE_TRUE = 0xB;
    const int BJSON_TYPE_NULL = 0xC;
    const int BJSON_TYPE_CDOCUMENT = 0x1F;

    extern std::vector<ubyte> to_binary(const dynamic::Map* obj);
    extern std::unique_ptr<dynamic::Map> from_binary(const ubyte* src, size_t size);
}

#endif // CODERS_BINARY_JSON_H_
