#include "binary_json.hpp"

#include "gzip.hpp"
#include "byte_utils.hpp"
#include "../data/dynamic.hpp"

#include <stdexcept>

using namespace json;
using namespace dynamic;

static void to_binary(ByteBuilder& builder, const Value* value) {
    switch (static_cast<valtype>(value->value.index())) {
        case valtype::none:
            throw std::runtime_error("none value is not implemented");
        case valtype::map: {
            std::vector<ubyte> bytes = to_binary(std::get<Map*>(value->value));
            builder.put(bytes.data(), bytes.size());
            break;
        }
        case valtype::list:
            builder.put(BJSON_TYPE_LIST);
            for (auto& element : std::get<List*>(value->value)->values) {
                to_binary(builder, element.get());
            }
            builder.put(BJSON_END);
            break;
        case valtype::integer: {
            auto val = std::get<integer_t>(value->value);
            if (val >= 0 && val <= 255) {
                builder.put(BJSON_TYPE_BYTE);
                builder.put(val);
            } else if (val >= INT16_MIN && val <= INT16_MAX){
                builder.put(BJSON_TYPE_INT16);
                builder.putInt16(val);
            } else if (val >= INT32_MIN && val <= INT32_MAX) {
                builder.put(BJSON_TYPE_INT32);
                builder.putInt32(val);
            } else {
                builder.put(BJSON_TYPE_INT64);
                builder.putInt64(val);
            }
            break;
        }
        case valtype::number:
            builder.put(BJSON_TYPE_NUMBER);
            builder.putFloat64(std::get<number_t>(value->value));
            break;
        case valtype::boolean:
            builder.put(BJSON_TYPE_FALSE + std::get<bool>(value->value));
            break;
        case valtype::string:
            builder.put(BJSON_TYPE_STRING);
            builder.put(std::get<std::string>(value->value));
            break;
    }
}

static std::unique_ptr<List> array_from_binary(ByteReader& reader);
static std::unique_ptr<Map> object_from_binary(ByteReader& reader);

std::vector<ubyte> json::to_binary(const Map* obj, bool compress) {
    if (compress) {
        auto bytes = to_binary(obj, false);
        return gzip::compress(bytes.data(), bytes.size());
    }
    ByteBuilder builder;
    // type byte
    builder.put(BJSON_TYPE_DOCUMENT);
    // document size
    builder.putInt32(0);

    // writing entries
    for (auto& entry : obj->values) {
        builder.putCStr(entry.first.c_str());
        to_binary(builder, entry.second.get());
    }
    // terminating byte
    builder.put(BJSON_END);

    // updating document size
    builder.setInt32(1, builder.size());
    return builder.build();
}

static std::unique_ptr<Value> value_from_binary(ByteReader& reader) {
    ubyte typecode = reader.get();
    valvalue val;
    switch (typecode) {
        case BJSON_TYPE_DOCUMENT:
            reader.getInt32();
            val = object_from_binary(reader).release();
            break;
        case BJSON_TYPE_LIST:
            val = array_from_binary(reader).release();
            break;
        case BJSON_TYPE_BYTE:
            val = static_cast<integer_t>(reader.get());
            break;
        case BJSON_TYPE_INT16:
            val = static_cast<integer_t>(reader.getInt16());
            break;
        case BJSON_TYPE_INT32:
            val = static_cast<integer_t>(reader.getInt32());
            break;
        case BJSON_TYPE_INT64:
            val = reader.getInt64();
            break;
        case BJSON_TYPE_NUMBER:
            val = reader.getFloat64();
            break;
        case BJSON_TYPE_FALSE:
        case BJSON_TYPE_TRUE:
            val = (typecode - BJSON_TYPE_FALSE) != 0;
            break;
        case BJSON_TYPE_STRING:
            val = reader.getString();
            break;
        default:
            throw std::runtime_error(
                "type "+std::to_string(typecode)+" is not supported"
            );
    }
    return std::make_unique<Value>(val);
}

static std::unique_ptr<List> array_from_binary(ByteReader& reader) {
    auto array = std::make_unique<List>();
    auto& items = array->values;
    while (reader.peek() != BJSON_END) {
        items.push_back(value_from_binary(reader));
    }
    reader.get();
    return array;
}

static std::unique_ptr<Map> object_from_binary(ByteReader& reader) {
    auto obj = std::make_unique<Map>();
    auto& map = obj->values;
    while (reader.peek() != BJSON_END) {
        const char* key = reader.getCString();
        map.insert(std::make_pair(key, value_from_binary(reader)));
    }
    reader.get();
    return obj;
}

std::unique_ptr<Map> json::from_binary(const ubyte* src, size_t size) {
    if (size < 2) {
        throw std::runtime_error("bytes length is less than 2");
    }
    if (src[0] == gzip::MAGIC[0] && src[1] == gzip::MAGIC[1]) {
        // reading compressed document
        auto data = gzip::decompress(src, size);
        return from_binary(data.data(), data.size());
    } else {
        ByteReader reader(src, size);
        std::unique_ptr<Value> value (value_from_binary(reader));

        if (Map* const* map = std::get_if<Map*>(&value->value)) {
            std::unique_ptr<Map> obj (*map);
            value->value = (Map*)nullptr;
            return obj;
        } else {
            throw std::runtime_error("root value is not an object");
        }
    }
}
