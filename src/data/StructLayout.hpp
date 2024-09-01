#pragma once

#include <vector>
#include <string>
#include <stdexcept>
#include <unordered_map>
#include <optional>

#include "typedefs.hpp"

namespace data {
    enum class FieldType {
        I8=0, I16, I32, I64, F32, F64, CHAR,
        COUNT
    };

    /// @brief Sorted by severity
    enum class FieldIncapatibilityType {
        NONE = 0,
        DATA_LOSS,
        TYPE_ERROR,
        MISSING,
    };

    struct FieldIncapatibility {
        std::string name;
        FieldIncapatibilityType type;
    };

    inline constexpr int sizeof_type(FieldType type) {
        const int sizes[static_cast<int>(FieldType::COUNT)] = {
            1, 2, 4, 8, 4, 8, 1
        };
        return sizes[static_cast<int>(type)];
    }

    class dataloss_error : public std::runtime_error {
    public:
        dataloss_error(const std::string& message) : std::runtime_error(message) {}
    };

    /// @brief Strategy will be used if value can't be left the same on conversion
    enum class FieldConvertStrategy {
        /// @brief Reset field value
        RESET,
        /// @brief Clamp field value if out of type range
        CLAMP
    };

    struct Field {
        FieldType type;
        std::string name;
        /// @brief Number of field elements (array size)
        int elements;
        /// @brief Strategy will be used in data loss case
        FieldConvertStrategy convertStrategy;
        /// @brief Byte offset of the field
        int offset;
        /// @brief Byte size of the field
        int size;
    };
    
    class StructLayout {
        int totalSize;
        std::vector<Field> fields;
        std::unordered_map<std::string, int> indices;
    public:
        StructLayout(
            int totalSize,
            std::vector<Field> fields,
            std::unordered_map<std::string, int> indices
        ) : totalSize(totalSize), 
            fields(std::move(fields)),
            indices(std::move(indices))
        {}

        /// @brief Get field by name. Returns nullptr if field not found.
        /// @param name field name
        /// @return nullable field pointer
        [[nodiscard]]
        const Field* getField(const std::string& name) const {
            auto found = indices.find(name);
            if (found == indices.end()) {
                return nullptr;
            }
            
            return &fields.at(found->second);
        }

        /// @brief Get field by name
        /// @throws std::runtime_exception - field not found
        /// @param name field name
        /// @return read-only field reference
        const Field& requreField(const std::string& name) const;

        /// @brief Get integer from specified field. 
        /// Types: (i8, i16, i32, i64, char)
        /// @throws std::runtime_exception - field not found
        /// @throws std::out_of_range - index is out of range [0, elements-1]
        /// @param src source buffer
        /// @param name field name
        /// @param index array index
        /// @return field value
        [[nodiscard]]
        integer_t getInteger(const ubyte* src, const std::string& name, int index=0) const;

        /// @brief Get floating-point number from specified field. 
        /// Types: (f32, f64, i8, i16, i32, i64, char)
        /// @throws std::runtime_exception - field not found
        /// @throws std::out_of_range - index is out of range [0, elements-1]
        /// @param src source buffer
        /// @param name field name
        /// @param index array index
        /// @return field value
        [[nodiscard]]
        number_t getNumber(const ubyte* src, const std::string& name, int index=0) const;

        /// @brief Get read-only chars array as string_view.
        /// @param src source buffer
        /// @param name field name
        [[nodiscard]]
        std::string_view getChars(const ubyte* src, const std::string& name) const;

        /// @brief Set field integer value.
        /// Types: (i8, i16, i32, i64, f32, f64, char)
        /// @throws std::runtime_exception - field not found
        /// @throws std::out_of_range - index is out of range [0, elements-1]
        /// @param dst destination buffer
        /// @param value value
        /// @param name field name
        /// @param index array index
        void setInteger(ubyte* dst, integer_t value, const std::string& name, int index=0) const;

        /// @brief Set field numeric value.
        /// Types: (f32, f64)
        /// @throws std::runtime_exception - field not found
        /// @throws std::out_of_range - index is out of range [0, elements-1]
        /// @param dst destination buffer
        /// @param value value
        /// @param name field name
        /// @param index array index
        void setNumber(ubyte* dst, number_t value, const std::string& name, int index=0) const;
        
        /// @brief Replace chars array to given ASCII string
        /// @throws std::runtime_exception - field not found
        /// @see StructMapper::setUnicode - utf-8 version of setAscii
        /// @param dst destination buffer
        /// @param value ASCII string
        /// @param name field name
        /// @return number of written string chars
        size_t setAscii(ubyte* dst, std::string_view value, const std::string& name) const;
        
        /// @brief Unicode-safe version of setAscii
        /// @throws std::runtime_exception - field not found
        /// @param dst destination buffer
        /// @param value utf-8 string
        /// @param name field name
        /// @return number of written string chars
        size_t setUnicode(ubyte* dst, std::string_view value, const std::string& name) const;

        /// @return total structure size (bytes)
        [[nodiscard]] size_t size() const {
            return totalSize;
        }

        /// @brief Convert structure data from srcLayout to this layout.
        /// @param srcLayout source structure layout
        /// @param src source data
        /// @param dst destination buffer
        /// (size must be enough to store converted structure) 
        /// @param allowDataLoss allow to drop fields that are not present in 
        /// this layout or have incompatible types
        /// @throws data::dataloss_error - data loss detected and allowDataLoss 
        /// is set to false
        void convert(
            const StructLayout& srcLayout, 
            const ubyte* src, 
            ubyte* dst,
            bool allowDataLoss) const;

        std::vector<FieldIncapatibility> checkCompatibility(
            const StructLayout& dstLayout);

        [[nodiscard]]
        static StructLayout create(const std::vector<Field>& fields);
    };
}