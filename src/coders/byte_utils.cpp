#include "byte_utils.hpp"

#include <cstring>
#include <limits>
#include <stdexcept>

#include "util/data_io.hpp"

ByteBuilder::ByteBuilder(size_t size) {
    buffer.reserve(size);
}

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

void ByteBuilder::putInt16(int16_t val, bool bigEndian) {
    size_t size = buffer.size();
    buffer.resize(buffer.size() + sizeof(int16_t));
    val = bigEndian ? dataio::h2be(val) : dataio::h2le(val);
    std::memcpy(buffer.data()+size, &val, sizeof(int16_t));
}

void ByteBuilder::putInt32(int32_t val, bool bigEndian) {
    size_t size = buffer.size();
    buffer.resize(buffer.size() + sizeof(int32_t));
    val = bigEndian ? dataio::h2be(val) : dataio::h2le(val);
    std::memcpy(buffer.data()+size, &val, sizeof(int32_t));
}

void ByteBuilder::putInt64(int64_t val, bool bigEndian) {
    size_t size = buffer.size();
    buffer.resize(buffer.size() + sizeof(int64_t));
    val = bigEndian ? dataio::h2be(val) : dataio::h2le(val);
    std::memcpy(buffer.data()+size, &val, sizeof(int64_t));
}

void ByteBuilder::putFloat32(float val, bool bigEndian) {
    int32_t i32_val;
    std::memcpy(&i32_val, &val, sizeof(int32_t));
    putInt32(i32_val, bigEndian);
}

void ByteBuilder::putFloat64(double val, bool bigEndian) {
    int64_t i64_val;
    std::memcpy(&i64_val, &val, sizeof(int64_t));
    putInt64(i64_val, bigEndian);
}

void ByteBuilder::set(size_t position, ubyte val) {
    buffer[position] = val;
}

void ByteBuilder::setInt16(size_t position, int16_t val) {
    val = dataio::h2le(val);
    std::memcpy(buffer.data()+position, &val, sizeof(int16_t));
}

void ByteBuilder::setInt32(size_t position, int32_t val) {
    val = dataio::h2le(val);
    std::memcpy(buffer.data()+position, &val, sizeof(int32_t));
}

void ByteBuilder::setInt64(size_t position, int64_t val) {
    val = dataio::h2le(val);
    std::memcpy(buffer.data()+position, &val, sizeof(int64_t));
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

ByteReader::ByteReader(const std::vector<ubyte>& data)
    : data(data.data()), size(data.size()), pos(0) {
}

void ByteReader::checkMagic(const char* data, size_t size) {
    if (pos + size >= this->size) {
        throw std::runtime_error("invalid magic number");
    }
    for (size_t i = 0; i < size; i++) {
        if (this->data[pos + i] != static_cast<ubyte>(data[i])) {
            throw std::runtime_error("invalid magic number");
        }
    }
    pos += size;
}

void ByteReader::get(char* dst, size_t size) {
    if (pos + size > this->size) {
        throw std::runtime_error("buffer underflow");
    }
    std::memcpy(dst, data+pos, size);
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

int16_t ByteReader::getInt16(bool bigEndian) {
    if (pos + sizeof(int16_t) > size) {
        throw std::runtime_error("buffer underflow");
    }
    int16_t value;
    std::memcpy(&value, data + pos, sizeof(int16_t));
    pos += sizeof(int16_t);
    return bigEndian ? dataio::be2h(value) : dataio::le2h(value);
}

int32_t ByteReader::getInt32(bool bigEndian) {
    if (pos + sizeof(int32_t) > size) {
        throw std::runtime_error("buffer underflow");
    }
    int32_t value;
    std::memcpy(&value, data + pos, sizeof(int32_t));
    pos += sizeof(int32_t);
    return bigEndian ? dataio::be2h(value) : dataio::le2h(value);
}

int64_t ByteReader::getInt64(bool bigEndian) {
    if (pos + sizeof(int64_t) > size) {
        throw std::runtime_error("buffer underflow");
    }
    int64_t value;
    std::memcpy(&value, data + pos, sizeof(int64_t));
    pos += sizeof(int64_t);
    return bigEndian ? dataio::be2h(value) : dataio::le2h(value);
}

float ByteReader::getFloat32(bool bigEndian) {
    int32_t i32_val = getInt32();
    if (bigEndian) {
        i32_val = dataio::be2h(i32_val);
    }
    float val;
    std::memcpy(&val, &i32_val, sizeof(float));
    return val;
}

double ByteReader::getFloat64(bool bigEndian) {
    int64_t i64_val = getInt64();
    if (bigEndian) {
        i64_val = dataio::be2h(i64_val);
    }
    double val;
    std::memcpy(&val, &i64_val, sizeof(double));
    return val;
}

const char* ByteReader::getCString() {
    const char* cstr = reinterpret_cast<const char*>(data + pos);
    pos += std::strlen(cstr) + 1;
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
