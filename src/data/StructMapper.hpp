#pragma once

#include <vector>
#include <string>
#include <unordered_map>

#include "typedefs.hpp"

namespace data {
    enum class FieldType {
        I8, I16, I32, I64, F32, F64
    };

    struct Field {
        FieldType type;
        std::string name;
        int offset;
    };
    
    class StructMapping {
        std::vector<Field> fields;
        std::unordered_map<std::string, int> indices;
    public:
        const Field* getField(const std::string& name) const {
            auto found = indices.find(name);
            if (found == indices.end()) {
                return nullptr;
            }
            return &fields.at(found->second);
        }
    };

    class StructAccess {
        const StructMapping& mapping;
        uint8_t* buffer;
    public:
        StructAccess(const StructMapping& mapping, uint8_t* buffer)
            : mapping(mapping), buffer(buffer) {
        }
    };
}
