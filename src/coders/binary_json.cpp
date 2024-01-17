#include "binary_json.h"

#include <stdexcept>

#include "gzip.h"
#include "byte_utils.h"

using namespace json;
using namespace dynamic;

static void to_binary(ByteBuilder& builder, const Value* value) {
    switch (value->type) {
        case valtype::map: {
            std::vector<ubyte> bytes = to_binary(value->value.map);
            builder.put(bytes.data(), bytes.size());
            break;
        }
        case valtype::list:
            builder.put(BJSON_TYPE_LIST);
            for (auto& element : value->value.list->values) {
                to_binary(builder, element.get());
            }
            builder.put(BJSON_END);
            break;
        case valtype::integer: {
            int64_t val = value->value.integer;
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
            builder.putFloat64(value->value.decimal);
            break;
        case valtype::boolean:
            builder.put(BJSON_TYPE_FALSE + value->value.boolean);
            break;
        case valtype::string:
            builder.put(BJSON_TYPE_STRING);
            builder.put(*value->value.str);
            break;
    }
}

static List* array_from_binary(ByteReader& reader);
static Map* object_from_binary(ByteReader& reader);

std::vector<ubyte> json::to_binary(const Map* obj) {
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

static Value* value_from_binary(ByteReader& reader) {
    ubyte typecode = reader.get();
    valtype type;
    valvalue val;
    switch (typecode) {
        case BJSON_TYPE_DOCUMENT:
            type = valtype::map;
            reader.getInt32();
            val.map = object_from_binary(reader);
            break;
        case BJSON_TYPE_LIST:
            type = valtype::list;
            val.list = array_from_binary(reader);
            break;
        case BJSON_TYPE_BYTE:
            type = valtype::integer;
            val.integer = reader.get();
            break;
        case BJSON_TYPE_INT16:
            type = valtype::integer;
            val.integer = reader.getInt16();
            break;
        case BJSON_TYPE_INT32:
            type = valtype::integer;
            val.integer = reader.getInt32();
            break;
        case BJSON_TYPE_INT64:
            type = valtype::integer;
            val.integer = reader.getInt64();
            break;
        case BJSON_TYPE_NUMBER:
            type = valtype::number;
            val.decimal = reader.getFloat64();
            break;
        case BJSON_TYPE_FALSE:
        case BJSON_TYPE_TRUE:
            type = valtype::boolean;
            val.boolean = typecode - BJSON_TYPE_FALSE;
            break;
        case BJSON_TYPE_STRING:
            type = valtype::string;
            val.str = new std::string(reader.getString());
            break;
        default:
            throw std::runtime_error(
                  "type "+std::to_string(typecode)+" is not supported");
    }
    return new Value(type, val);
}

static List* array_from_binary(ByteReader& reader) {
    auto array = std::make_unique<List>();
    auto& items = array->values;
    while (reader.peek() != BJSON_END) {
        items.push_back(std::unique_ptr<Value>(value_from_binary(reader)));
    }
    reader.get();
    return array.release();
}

static Map* object_from_binary(ByteReader& reader) {
    auto obj = std::make_unique<Map>();
    auto& map = obj->values;
    while (reader.peek() != BJSON_END) {
        const char* key = reader.getCString();
        Value* value = value_from_binary(reader);
        map.insert(std::make_pair(key, value));
    }
    reader.get();
    return obj.release();
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
        if (value->type != valtype::map) {
            throw std::runtime_error("root value is not an object");
        }
        std::unique_ptr<Map> obj (value->value.map);
        value->value.map = nullptr;
        return obj;
    }
}
