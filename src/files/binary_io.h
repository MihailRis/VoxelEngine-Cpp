#ifndef FILES_BINARY_WRITER_H_
#define FILES_BINARY_WRITER_H_

#include <string>
#include <vector>
#include "../typedefs.h"

class BinaryWriter {
    std::vector<u_char8> buffer;
public:
    void put(u_char8 b);
    void putCStr(const char* str);
    void putInt16(int16_t val);
    void putInt32(int32_t val);
    void putInt64(int64_t val);
    void putFloat32(float val);
    void put(const std::string& s);
    void put(const u_char8* arr, size_t size);
    void putShortStr(const std::string& s);

    inline size_t size() const {
        return buffer.size();
    }
    inline const u_char8* data() const {
        return buffer.data();
    }
};

class BinaryReader {
    const u_char8* data;
    size_t size;
    size_t pos;
public:
    BinaryReader(const u_char8* data, size_t size);

    void checkMagic(const char* data, size_t size);
    u_char8 get();
    int16_t getInt16();
    int32_t getInt32();
    int64_t getInt64();
    float getFloat32();
    std::string getString();
    std::string getShortString();
    bool hasNext() const;
};

#endif // FILES_BINARY_WRITER_H_