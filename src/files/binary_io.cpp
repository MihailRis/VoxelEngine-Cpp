#include "binary_io.h"

#include <string.h>
#include <limits>
#include <stdexcept>

using std::string;

void BinaryWriter::put(ubyte b) {
    buffer.push_back(b);
}

void BinaryWriter::putCStr(const char* str) {
    size_t size = strlen(str)+1;
    buffer.reserve(buffer.size() + size);
    for (size_t i = 0; i < size; i++) {
        buffer.push_back(str[i]);
    }
}

void BinaryWriter::put(const string& s) {
    size_t len = s.length();
    if (len > INT16_MAX) {
        throw std::domain_error("length > INT16_MAX");
    }
    putInt16(len);
    put((const ubyte*)s.data(), len);
}

void BinaryWriter::put(const ubyte* arr, size_t size) {
    buffer.reserve(buffer.size() + size);
    for (size_t i = 0; i < size; i++) {
        buffer.push_back(arr[i]);
    }
}

void BinaryWriter::putInt16(int16_t val) {
    buffer.push_back((char) (val >> 8 & 255));
    buffer.push_back((char) (val >> 0 & 255));
}

void BinaryWriter::putInt32(int32_t val) {
    buffer.reserve(buffer.size() + 4);
    buffer.push_back((char) (val >> 24 & 255));
    buffer.push_back((char) (val >> 16 & 255));
    buffer.push_back((char) (val >> 8 & 255));
    buffer.push_back((char) (val >> 0 & 255));
}

void BinaryWriter::putInt64(int64_t val) {
    buffer.reserve(buffer.size() + 8);
    buffer.push_back((char) (val >> 56 & 255));
    buffer.push_back((char) (val >> 48 & 255));
    buffer.push_back((char) (val >> 40 & 255));
    buffer.push_back((char) (val >> 32 & 255));
    buffer.push_back((char) (val >> 24 & 255));
    buffer.push_back((char) (val >> 16 & 255));
    buffer.push_back((char) (val >> 8 & 255));
    buffer.push_back((char) (val >> 0 & 255));
}

void BinaryWriter::putFloat32(float val) {
    putInt32(*((uint32_t*)&val));
}

BinaryReader::BinaryReader(const ubyte* data, size_t size)
    : data(data), size(size), pos(0) {
}

void BinaryReader::checkMagic(const char* data, size_t size) {
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

ubyte BinaryReader::get() {
    if (pos == size) {
        throw std::underflow_error("buffer underflow");
    }
    return data[pos++];
}

int16_t BinaryReader::getInt16() {
    if (pos+2 >= size) {
        throw std::underflow_error("unexpected end");
    }
    pos += 2;
    return (data[pos - 2] << 8) | 
           (data[pos - 1]);
}

int32_t BinaryReader::getInt32() {
    if (pos+4 >= size) {
        throw std::underflow_error("unexpected end");
    }
    pos += 4;
    return (data[pos - 4] << 24) | 
           (data[pos - 3] << 16) | 
           (data[pos - 2] << 8) | 
           (data[pos - 1]);
}

int64_t BinaryReader::getInt64() {
    if (pos+8 >= size) {
        throw std::underflow_error("unexpected end");
    }
    pos += 8;
    return ((int64_t)data[pos - 8] << 56) | 
           ((int64_t)data[pos - 7] << 48) | 
           ((int64_t)data[pos - 6] << 40) | 
           ((int64_t)data[pos - 5] << 32) | 
           ((int64_t)data[pos - 4] << 24) | 
           ((int64_t)data[pos - 3] << 16) | 
           ((int64_t)data[pos - 2] << 8) | 
           ((int64_t)data[pos - 1]);
}

float BinaryReader::getFloat32() {
    int32_t value = getInt32();
    return *(float*)(&value);
}

string BinaryReader::getString() {
    uint16_t length = (uint16_t)getInt16();
    if (pos+length >= size) {
        throw std::underflow_error("unexpected end");
    }
    pos += length;
    return string((const char*)(data+pos-length), length);
}

bool BinaryReader::hasNext() const {
    return pos < size;
}
