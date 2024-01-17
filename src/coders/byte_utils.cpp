#include "byte_utils.h"

#include <cstring>
#include <limits>
#include <stdexcept>

void ByteBuilder::put(ubyte b) {
    buffer.push_back(b);
}

void ByteBuilder::putCStr(const char* str) {
    size_t size = strlen(str)+1;
    buffer.reserve(buffer.size() + size);
    for (size_t i = 0; i < size; i++) {
        buffer.push_back(str[i]);
    }
}

void ByteBuilder::put(const std::string& s) {
    size_t len = s.length();
    putInt32(len);
    put((const ubyte*)s.data(), len);
}

void ByteBuilder::put(const ubyte* arr, size_t size) {
    buffer.reserve(buffer.size() + size);
    for (size_t i = 0; i < size; i++) {
        buffer.push_back(arr[i]);
    }
}

void ByteBuilder::putInt16(int16_t val) {
    buffer.push_back((char) (val >> 0 & 255));
    buffer.push_back((char) (val >> 8 & 255));
}

void ByteBuilder::putInt32(int32_t val) {
    buffer.reserve(buffer.size() + 4);
    buffer.push_back((char) (val >> 0 & 255));
    buffer.push_back((char) (val >> 8 & 255));
    buffer.push_back((char) (val >> 16 & 255));
    buffer.push_back((char) (val >> 24 & 255));
}

void ByteBuilder::putInt64(int64_t val) {
    buffer.reserve(buffer.size() + 8);
    buffer.push_back((char) (val >> 0 & 255));
    buffer.push_back((char) (val >> 8 & 255));
    buffer.push_back((char) (val >> 16 & 255));
    buffer.push_back((char) (val >> 24 & 255));
    buffer.push_back((char) (val >> 32 & 255));
    buffer.push_back((char) (val >> 40 & 255));
    buffer.push_back((char) (val >> 48 & 255));
    buffer.push_back((char) (val >> 56 & 255));
}

void ByteBuilder::putFloat32(float val) {
    union {
        int32_t vali32;
        float valfloat;
    } value;
    value.valfloat = val;
    putInt32(value.vali32);
}

void ByteBuilder::putFloat64(double val) {
    union {
        int64_t vali64;
        double valfloat;
    } value;
    value.valfloat = val;
    putInt64(value.vali64);
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

ByteReader::ByteReader(const ubyte* data)
    : data(data), size(4), pos(0) {
    size = getInt32();
}

void ByteReader::checkMagic(const char* data, size_t size) {
    if (pos + size >= this->size) {
        throw std::runtime_error("invalid magic number");
    }
    for (size_t i = 0; i < size; i++) {
        if (this->data[pos + i] != (ubyte)data[i]){
            throw std::runtime_error("invalid magic number");
        }
    }
    pos += size;
}

ubyte ByteReader::get() {
    if (pos == size) {
        throw std::underflow_error("buffer underflow");
    }
    return data[pos++];
}

ubyte ByteReader::peek() {
    if (pos == size) {
        throw std::underflow_error("buffer underflow");
    }
    return data[pos]; 
}

int16_t ByteReader::getInt16() {
    if (pos+2 > size) {
        throw std::underflow_error("unexpected end");
    }
    pos += 2;
    return (data[pos - 1] << 8) | 
           (data[pos - 2]);
}

int32_t ByteReader::getInt32() {
    if (pos+4 > size) {
        throw std::underflow_error("unexpected end");
    }
    pos += 4;
    return (data[pos - 1] << 24) | 
           (data[pos - 2] << 16) | 
           (data[pos - 3] << 8) | 
           (data[pos - 4]);
}

int64_t ByteReader::getInt64() {
    if (pos+8 > size) {
        throw std::underflow_error("unexpected end");
    }
    pos += 8;
    return ((int64_t)data[pos - 1] << 56) | 
           ((int64_t)data[pos - 2] << 48) | 
           ((int64_t)data[pos - 3] << 40) | 
           ((int64_t)data[pos - 4] << 32) | 
           ((int64_t)data[pos - 5] << 24) | 
           ((int64_t)data[pos - 6] << 16) | 
           ((int64_t)data[pos - 7] << 8) | 
           ((int64_t)data[pos - 8]);
}

float ByteReader::getFloat32() {
    union {
        int32_t vali32;
        float valfloat;
    } value;
    value.vali32 = getInt32();
    return value.valfloat;
}

double ByteReader::getFloat64() {
    union {
        int64_t vali64;
        double valfloat;
    } value;
    value.vali64 = getInt64();
    return value.valfloat;
}

const char* ByteReader::getCString() {
    const char* cstr = (const char*)(data+pos);
    pos += strlen(cstr) + 1;
    return cstr;
}

std::string ByteReader::getString() {
    uint32_t length = (uint32_t)getInt32();
    if (pos+length > size) {
        throw std::underflow_error("unexpected end");
    }
    pos += length;
    return std::string((const char*)(data+pos-length), length);
}

bool ByteReader::hasNext() const {
    return pos < size;
}
