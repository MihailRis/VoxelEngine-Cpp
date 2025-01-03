#pragma once

#include <string>
#include <vector>

#include "typedefs.hpp"

class ByteBuilder {
    std::vector<ubyte> buffer;
public:
    ByteBuilder() = default;
    ByteBuilder(size_t size);

    /// @brief Write one byte (8 bit unsigned integer)
    void put(ubyte b);
    /// @brief Write c-string (bytes array terminated with '\00')
    void putCStr(const char* str);
    /// @brief Write signed 16 bit little-endian integer
    void putInt16(int16_t val, bool bigEndian = false);
    /// @brief Write signed 32 bit integer
    void putInt32(int32_t val, bool bigEndian = false);
    /// @brief Write signed 64 bit integer
    void putInt64(int64_t val, bool bigEndian = false);
    /// @brief Write 32 bit floating-point number
    void putFloat32(float val, bool bigEndian = false);
    /// @brief Write 64 bit floating-point number
    void putFloat64(double val, bool bigEndian = false);

    /// @brief Write string (uint32 length + bytes)
    void put(const std::string& s);
    /// @brief Write sequence of bytes without any header
    void put(const ubyte* arr, size_t size);

    void set(size_t position, ubyte val);
    void setInt16(size_t position, int16_t val);
    void setInt32(size_t position, int32_t val);
    void setInt64(size_t position, int64_t val);

    inline size_t size() const {
        return buffer.size();
    }
    inline const ubyte* data() const {
        return buffer.data();
    }

    std::vector<ubyte> build();
};

class ByteReader {
    const ubyte* data;
    size_t size;
    size_t pos;
public:
    ByteReader(const ubyte* data, size_t size);
    ByteReader(const ubyte* data);
    ByteReader(const std::vector<ubyte>& data);

    void checkMagic(const char* data, size_t size);
    /// @brief Get N bytes
    void get(char* dst, size_t size);
    /// @brief Read one byte (unsigned 8 bit integer)
    ubyte get();
    /// @brief Read one byte (unsigned 8 bit integer) without pointer move
    ubyte peek();
    /// @brief Read signed 16 bit little-endian integer
    int16_t getInt16(bool bigEndian = false);
    /// @brief Read signed 32 bit little-endian integer
    int32_t getInt32(bool bigEndian = false);
    /// @brief Read signed 64 bit little-endian integer
    int64_t getInt64(bool bigEndian = false);
    /// @brief Read 32 bit floating-point number
    float getFloat32(bool bigEndian = false);
    /// @brief Read 64 bit floating-point number
    double getFloat64(bool bigEndian = false);
    /// @brief Read C-String
    const char* getCString();
    /// @brief Read string with unsigned 32 bit number before (length)
    std::string getString();
    /// @return true if there is at least one byte remains
    bool hasNext() const;
    /// @return Number of remaining bytes in buffer
    size_t remaining() const;

    const ubyte* pointer() const;
    void skip(size_t n);
};
