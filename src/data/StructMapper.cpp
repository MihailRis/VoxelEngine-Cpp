#include "StructMapper.hpp"

#include <cstring>
#include <stdexcept>
#include <algorithm>

#include "util/data_io.hpp"

using namespace data;

static_assert(sizeof(float) == sizeof(int32_t));
static_assert(sizeof(double) == sizeof(int64_t));

StructMapping StructMapping::create(const std::vector<Field>& fields) {
    std::vector<Field> builtFields = fields;
    std::unordered_map<std::string, int> indices;

    for (Field& field : builtFields) {
        field.size = sizeof_type(field.type) * field.elements;
    }
    std::sort(builtFields.begin(), builtFields.end(), 
        [](const Field& a, const Field& b) {
            return a.size > b.size;
        }
    );
    int offset = 0;
    for (int i = 0; i < builtFields.size(); i++) {
        auto& field = builtFields[i];
        field.offset = offset;
        indices[field.name] = i;
        offset += field.size;
    }
    return StructMapping(
        offset, std::move(builtFields), std::move(indices));
}

const Field& StructMapping::requreField(const std::string& name) const {
    auto found = indices.find(name);
    if (found == indices.end()) {
        throw std::runtime_error("field '"+name+"' does not exist");
    }
    return *&fields.at(found->second);
}


template<typename T>
static void set_int(ubyte* dst, integer_t value) {
    T out_value = static_cast<T>(value);
    out_value = dataio::le2h(out_value);
    *reinterpret_cast<T*>(dst) = out_value;
}

void StructMapping::setInteger(
    ubyte* dst, integer_t value, const std::string& name, int index
) {
    const auto& field = requreField(name);
    if (index < 0 || index >= field.elements) {
        throw std::runtime_error(
            "index out of bounds [0, "+std::to_string(field.elements)+"]");
    }
    auto ptr = dst + field.offset + index * sizeof_type(field.type);
    switch (field.type) {
        case FieldType::I8: set_int<int8_t>(ptr, value); break;
        case FieldType::I16: set_int<int16_t>(ptr, value); break;
        case FieldType::I32: set_int<int32_t>(ptr, value); break;
        case FieldType::I64: set_int<int64_t>(ptr, value); break;
        case FieldType::CHAR: set_int<int8_t>(ptr, value); break;
        case FieldType::F32:
        case FieldType::F64:
            setNumber(dst, static_cast<number_t>(value), name, index);
            break;
        default: 
            throw std::runtime_error("type error");
    }
}

void StructMapping::setNumber(
    ubyte* dst, number_t value, const std::string& name, int index
) {
    const auto& field = requreField(name);
    if (index < 0 || index >= field.elements) {
        throw std::runtime_error(
            "index out of bounds [0, "+std::to_string(field.elements)+"]");
    }
    auto ptr = dst + field.offset + index * sizeof_type(field.type);
    switch (field.type) {
        case FieldType::F32: {
            float fval = static_cast<float>(value);
            int32_t ival;
            std::memcpy(&ival, &fval, sizeof(int32_t));
            set_int<int32_t>(ptr, ival);
            break;
        }
        case FieldType::F64: {
            double fval = static_cast<double>(value);
            int64_t ival;
            std::memcpy(&ival, &fval, sizeof(int64_t));
            set_int<int64_t>(ptr, ival);
            break;
        }
        default:
            throw std::runtime_error("type error");
    }
}

template<typename T>
static T get_int(const ubyte* src) {
    return dataio::le2h(*reinterpret_cast<const T*>(src));
}

integer_t StructMapping::getInteger(
    const ubyte* src, const std::string& name, int index
) const {
    const auto& field = requreField(name);
    if (index < 0 || index >= field.elements) {
        throw std::runtime_error(
            "index out of bounds [0, "+std::to_string(field.elements)+"]");
    }
    auto ptr = src + field.offset + index * sizeof_type(field.type);
    switch (field.type) {
        case FieldType::I8: return get_int<int8_t>(ptr);
        case FieldType::I16: return get_int<int16_t>(ptr);
        case FieldType::I32: return get_int<int32_t>(ptr);
        case FieldType::I64: return get_int<int64_t>(ptr);
        case FieldType::CHAR: return get_int<int8_t>(ptr);
        default:
            throw std::runtime_error("type error");
    }
}

number_t StructMapping::getNumber(
    const ubyte* src, const std::string& name, int index
) const {
    const auto& field = requreField(name);
    if (index < 0 || index >= field.elements) {
        throw std::runtime_error(
            "index out of bounds [0, "+std::to_string(field.elements)+"]");
    }
    auto ptr = src + field.offset + index * sizeof_type(field.type);
    switch (field.type) {
        case FieldType::F32: {
            float fval;
            auto ival = get_int<int32_t>(ptr);
            std::memcpy(&fval, &ival, sizeof(float));
            return fval;
        }
        case FieldType::F64: {
            double fval;
            auto ival = get_int<int64_t>(ptr);
            std::memcpy(&fval, &ival, sizeof(double));
            return fval;
        }
        case FieldType::I8:
        case FieldType::I16:
        case FieldType::I32:
        case FieldType::I64:
        case FieldType::CHAR:
            return getInteger(src, name, index);
        
    }
    throw std::runtime_error("type error");
}
