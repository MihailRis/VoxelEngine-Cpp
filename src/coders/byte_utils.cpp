#include "byte_utils.hpp"

#include <cstring>
#include <limits>
#include <stdexcept>

void ByteBuilder::put(ubyte b) {
    buffer.push_back(b);
}

void ByteBuilder::putCStr(const char* str) {
    size_t size = std::strlen(str) + 1;
    buffer.reserve(buffer.size() + size);
    for (size_t i = 0; i < size; i++) {
        buffer.push_back(str[i]);
    }
}

void ByteBuilder::put(const std::string& s) {
    size_t len = s.length();
    putInt32(len);
    put(reinterpret_cast<const ubyte*>(s.data()), len);
}

void ByteBuilder::put(const ubyte* arr, size_t size) {
    buffer.reserve(buffer.size() + size);
    for (size_t i = 0; i < size; i++) {
        buffer.push_back(arr[i]);
    }
}

void ByteBuilder::putInt16(int16_t val) {
    buffer.push_back(static_cast<ubyte>(val >> 0 & 255));
    buffer.push_back(static_cast<ubyte>(val >> 8 & 255));
}

void ByteBuilder::putInt32(int32_t val) {
    buffer.reserve(buffer.size() + 4);
    buffer.push_back(static_cast<ubyte>(val >> 0 & 255));
    buffer.push_back(static_cast<ubyte>(val >> 8 & 255));
    buffer.push_back(static_cast<ubyte>(val >> 16 & 255));
    buffer.push_back(static_cast<ubyte>(val >> 24 & 255));
}

void ByteBuilder::putInt64(int64_t val) {
    buffer.reserve(buffer.size() + 8);
    buffer.push_back(static_cast<ubyte>(val >> 0 & 255));
    buffer.push_back(static_cast<ubyte>(val >> 8 & 255));
    buffer.push_back(static_cast<ubyte>(val >> 16 & 255));
    buffer.push_back(static_cast<ubyte>(val >> 24 & 255));
    buffer.push_back(static_cast<ubyte>(val >> 32 & 255));
    buffer.push_back(static_cast<ubyte>(val >> 40 & 255));
    buffer.push_back(static_cast<ubyte>(val >> 48 & 255));
    buffer.push_back(static_cast<ubyte>(val >> 56 & 255));
}

void ByteBuilder::putFloat32(float val) {
    int32_t i32_val;
    std::memcpy(&i32_val, &val, sizeof(int32_t));
    putInt32(i32_val);
}

void ByteBuilder::putFloat64(double val) {
    int64_t i64_val;
    std::memcpy(&i64_val, &val, sizeof(int64_t));
    putInt64(i64_val);
}

void ByteBuilder::set(size_t position, ubyte val) {
    buffer[position] = val;
}

void ByteBuilder::setInt16(size_t position, int16_t val) {
    buffer[position++] = val >> 0 & 255;
    buffer[position] = val >> 8 & 255;
}

void ByteBuilder::setInt32(size_t position, int32_t val) {
    buffer[position++] = val >> 0 & 255;
    buffer[position++] = val >> 8 & 255;
    buffer[position++] = val >> 16 & 255;
    buffer[position] = val >> 24 & 255;
}

void ByteBuilder::setInt64(size_t position, int64_t val) {
    buffer[position++] = val >> 0 & 255;
    buffer[position++] = val >> 8 & 255;
    buffer[position++] = val >> 16 & 255;
    buffer[position++] = val >> 24 & 255;

    buffer[position++] = val >> 32 & 255;
    buffer[position++] = val >> 40 & 255;
    buffer[position++] = val >> 48 & 255;
    buffer[position] = val >> 56 & 255;
}

std::vector<ubyte> ByteBuilder::build() {
    return buffer;
}

ByteReader::ByteReader(const ubyte* data, size_t size)
    : data(data), size(size), pos(0) {
}

ByteReader::ByteReader(const ubyte* data) : data(data), size(4), pos(0) {
    size = getInt32();
}

void ByteReader::checkMagic(const char* data, size_t size) {
    if (pos + size >= this->size) {
        throw std::runtime_error("invalid magic number");
    }
    for (size_t i = 0; i < size; i++) {
        if (this->data[pos + i] != (ubyte)data[i]) {
            throw std::runtime_error("invalid magic number");
        }
    }
    pos += size;
}

ubyte ByteReader::get() {
    if (pos == size) {
        throw std::runtime_error("buffer underflow");
    }
    return data[pos++];
}

ubyte ByteReader::peek() {
    if (pos == size) {
        throw std::runtime_error("buffer underflow");
    }
    return data[pos];
}

int16_t ByteReader::getInt16() {
    if (pos + 2 > size) {
        throw std::runtime_error("buffer underflow");
    }
    pos += 2;
    return (static_cast<int16_t>(data[pos - 1]) << 8) |
           (static_cast<int16_t>(data[pos - 2]));
}

int32_t ByteReader::getInt32() {
    if (pos + 4 > size) {
        throw std::runtime_error("buffer underflow");
    }
    pos += 4;
    return (static_cast<int32_t>(data[pos - 1]) << 24) |
           (static_cast<int32_t>(data[pos - 2]) << 16) |
           (static_cast<int32_t>(data[pos - 3]) << 8) |
           (static_cast<int32_t>(data[pos - 4]));
}

int64_t ByteReader::getInt64() {
    if (pos + 8 > size) {
        throw std::runtime_error("buffer underflow");
    }
    pos += 8;
    return (static_cast<int64_t>(data[pos - 1]) << 56) |
           (static_cast<int64_t>(data[pos - 2]) << 48) |
           (static_cast<int64_t>(data[pos - 3]) << 40) |
           (static_cast<int64_t>(data[pos - 4]) << 32) |
           (static_cast<int64_t>(data[pos - 5]) << 24) |
           (static_cast<int64_t>(data[pos - 6]) << 16) |
           (static_cast<int64_t>(data[pos - 7]) << 8) |
           (static_cast<int64_t>(data[pos - 8]));
}

float ByteReader::getFloat32() {
    int32_t i32_val = getInt32();
    float val;
    std::memcpy(&val, &i32_val, sizeof(float));
    return val;
}

double ByteReader::getFloat64() {
    int64_t i64_val = getInt64();
    double val;
    std::memcpy(&val, &i64_val, sizeof(double));
    return val;
}

const char* ByteReader::getCString() {
    const char* cstr = reinterpret_cast<const char*>(data + pos);
    pos += strlen(cstr) + 1;
    return cstr;
}

std::string ByteReader::getString() {
    uint32_t length = static_cast<uint32_t>(getInt32());
    if (pos + length > size) {
        throw std::runtime_error("buffer underflow");
    }
    pos += length;
    return std::string(
        reinterpret_cast<const char*>(data + pos - length), length
    );
}

bool ByteReader::hasNext() const {
    return pos < size;
}

size_t ByteReader::remaining() const {
    return size - pos;
}

const ubyte* ByteReader::pointer() const {
    return data + pos;
}

void ByteReader::skip(size_t n) {
    pos += n;
}
