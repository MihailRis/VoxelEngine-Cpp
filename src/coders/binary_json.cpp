#include "binary_json.hpp"

#include <stdexcept>

#include "data/dynamic.hpp"
#include "byte_utils.hpp"
#include "gzip.hpp"

using namespace json;
using namespace dynamic;

static void to_binary(ByteBuilder& builder, const Value& value) {
    switch (static_cast<Type>(value.index())) {
        case Type::none:
            throw std::runtime_error("none value is not implemented");
        case Type::map: {
            const auto bytes = to_binary(std::get<Map_sptr>(value).get());
            builder.put(bytes.data(), bytes.size());
            break;
        }
        case Type::list:
            builder.put(BJSON_TYPE_LIST);
            for (const auto& element : std::get<List_sptr>(value)->values) {
                to_binary(builder, element);
            }
            builder.put(BJSON_END);
            break;
        case Type::bytes: {
            const auto bytes = std::get<ByteBuffer_sptr>(value).get();
            builder.put(BJSON_TYPE_BYTES);
            builder.putInt32(bytes->size());
            builder.put(bytes->data(), bytes->size());
            break;
        }
        case Type::integer: {
            auto val = std::get<integer_t>(value);
            if (val >= 0 && val <= 255) {
                builder.put(BJSON_TYPE_BYTE);
                builder.put(val);
            } else if (val >= INT16_MIN && val <= INT16_MAX) {
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
        case Type::number:
            builder.put(BJSON_TYPE_NUMBER);
            builder.putFloat64(std::get<number_t>(value));
            break;
        case Type::boolean:
            builder.put(BJSON_TYPE_FALSE + std::get<bool>(value));
            break;
        case Type::string:
            builder.put(BJSON_TYPE_STRING);
            builder.put(std::get<std::string>(value));
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
        to_binary(builder, entry.second);
    }
    // terminating byte
    builder.put(BJSON_END);

    // updating document size
    builder.setInt32(1, builder.size());
    return builder.build();
}

std::vector<ubyte> json::to_binary(const Value& value, bool compress) {
    if (auto map = std::get_if<Map_sptr>(&value)) {
        return to_binary(map->get(), compress);
    }
    throw std::runtime_error("map is only supported as the root element");
}

static Value value_from_binary(ByteReader& reader) {
    ubyte typecode = reader.get();
    switch (typecode) {
        case BJSON_TYPE_DOCUMENT:
            reader.getInt32();
            return Map_sptr(object_from_binary(reader).release());
        case BJSON_TYPE_LIST:
            return List_sptr(array_from_binary(reader).release());
        case BJSON_TYPE_BYTE:
            return static_cast<integer_t>(reader.get());
        case BJSON_TYPE_INT16:
            return static_cast<integer_t>(reader.getInt16());
        case BJSON_TYPE_INT32:
            return static_cast<integer_t>(reader.getInt32());
        case BJSON_TYPE_INT64:
            return reader.getInt64();
        case BJSON_TYPE_NUMBER:
            return reader.getFloat64();
        case BJSON_TYPE_FALSE:
        case BJSON_TYPE_TRUE:
            return (typecode - BJSON_TYPE_FALSE) != 0;
        case BJSON_TYPE_STRING:
            return reader.getString();
        case BJSON_TYPE_NULL:
            return NONE;
        case BJSON_TYPE_BYTES: {
            int32_t size = reader.getInt32();
            if (size < 0) {
                throw std::runtime_error(
                    "invalid byte-buffer size "+std::to_string(size));
            }
            if (size > reader.remaining()) {
                throw std::runtime_error(
                    "buffer_size > remaining_size "+std::to_string(size));
            }
            return std::make_shared<ByteBuffer>(reader.pointer(), size);
        }
    }
    throw std::runtime_error(
        "type support not implemented for <"+std::to_string(typecode)+">");
}

static std::unique_ptr<List> array_from_binary(ByteReader& reader) {
    auto array = std::make_unique<List>();
    while (reader.peek() != BJSON_END) {
        array->put(value_from_binary(reader));
    }
    reader.get();
    return array;
}

static std::unique_ptr<Map> object_from_binary(ByteReader& reader) {
    auto obj = std::make_unique<Map>();
    while (reader.peek() != BJSON_END) {
        const char* key = reader.getCString();
        obj->put(key, value_from_binary(reader));
    }
    reader.get();
    return obj;
}

std::shared_ptr<Map> json::from_binary(const ubyte* src, size_t size) {
    if (size < 2) {
        throw std::runtime_error("bytes length is less than 2");
    }
    if (src[0] == gzip::MAGIC[0] && src[1] == gzip::MAGIC[1]) {
        // reading compressed document
        auto data = gzip::decompress(src, size);
        return from_binary(data.data(), data.size());
    } else {
        ByteReader reader(src, size);
        Value value = value_from_binary(reader);

        if (auto map = std::get_if<Map_sptr>(&value)) {
            return *map;
        } else {
            throw std::runtime_error("root value is not an object");
        }
    }
}
