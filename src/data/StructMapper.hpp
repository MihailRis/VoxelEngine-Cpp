#pragma once

#include <vector>
#include <string>
#include <unordered_map>

#include "typedefs.hpp"

namespace data {
    enum class FieldType {
        I8=0, I16, I32, I64, F32, F64, CHAR,
        COUNT
    };

    inline constexpr int sizeof_type(FieldType type) {
        const int sizes[static_cast<int>(FieldType::COUNT)] = {
            1, 2, 4, 8, 4, 8, 1
        };
        return sizes[static_cast<int>(type)];
    }

    struct Field {
        FieldType type;
        std::string name;
        /// @brief Number of field elements (array size)
        int elements;
        /// @brief Byte offset of the field
        int offset;
        /// @brief Byte size of the field
        int size;
    };
    
    class StructMapping {
        int totalSize;
        std::vector<Field> fields;
        std::unordered_map<std::string, int> indices;
    public:
        StructMapping(
            int totalSize,
            std::vector<Field> fields,
            std::unordered_map<std::string, int> indices
        ) : totalSize(totalSize), 
            fields(std::move(fields)),
            indices(std::move(indices))
        {}

        const Field* getField(const std::string& name) const {
            auto found = indices.find(name);
            if (found == indices.end()) {
                return nullptr;
            }
            
            return &fields.at(found->second);
        }

        const Field& requreField(const std::string& name) const;

        integer_t getInteger(const ubyte* src, const std::string& name, int index=0) const;
        number_t getNumber(const ubyte* src, const std::string& name, int index=0) const;
        std::string_view getChars(const ubyte* src, const std::string& name) const;

        void setInteger(ubyte* dst, integer_t value, const std::string& name, int index=0);
        void setNumber(ubyte* dst, number_t value, const std::string& name, int index=0);
        void setChars(ubyte* dst, std::string_view value, const std::string& name);

        int size() const {
            return totalSize;
        }

        static StructMapping create(const std::vector<Field>& fields);
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
