#ifndef CODERS_BYTE_UTILS_HPP_
#define CODERS_BYTE_UTILS_HPP_

#include <string>
#include <vector>

#include <typedefs.hpp>

/* byteorder: little-endian */
class ByteBuilder {
    std::vector<ubyte> buffer;
public:
    /* Write one byte (8 bit unsigned integer) */
    void put(ubyte b);
    /* Write c-string (bytes array terminated with '\00') */
    void putCStr(const char* str);
    /* Write signed 16 bit integer */
    void putInt16(int16_t val);
    /* Write signed 32 bit integer */
    void putInt32(int32_t val);
    /* Write signed 64 bit integer */
    void putInt64(int64_t val);
    /* Write 32 bit floating-point number */
    void putFloat32(float val);
    /* Write 64 bit floating-point number */
    void putFloat64(double val);

    /* Write string (uint32 length + bytes) */
    void put(const std::string& s);
    /* Write sequence of bytes without any header */
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

/// byteorder: little-endian
class ByteReader {
    const ubyte* data;
    size_t size;
    size_t pos;
public:
    ByteReader(const ubyte* data, size_t size);
    ByteReader(const ubyte* data);

    void checkMagic(const char* data, size_t size);
    /// @brief Read one byte (unsigned 8 bit integer)
    ubyte get();
    /// @brief Read one byte (unsigned 8 bit integer) without pointer move
    ubyte peek();
    /// @brief Read signed 16 bit integer
    int16_t getInt16();
    /// @brief Read signed 32 bit integer
    int32_t getInt32();
    /// @brief Read signed 64 bit integer
    int64_t getInt64();
    /// @brief Read 32 bit floating-point number
    float getFloat32();
    /// @brief Read 64 bit floating-point number
    double getFloat64();
    /// @brief Read C-String
    const char* getCString();
    /// @brief Read string with unsigned 32 bit number before (length)
    std::string getString();
    /// @return true if there is at least one byte remains
    bool hasNext() const;

    const ubyte* pointer() const;
    void skip(size_t n);
};

#endif  // CODERS_BYTE_UTILS_HPP_
