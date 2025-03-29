#pragma once

#include <istream>
#include "util/Buffer.hpp"

class memory_streambuf : public std::streambuf {
public:
    explicit memory_streambuf(util::Buffer<char> buffer)
        : buffer(std::move(buffer)) {
        char* base = this->buffer.data();
        char* end = base + this->buffer.size();
        setg(base, base, end);
    }
    
    memory_streambuf(const memory_streambuf&) = delete;
    memory_streambuf& operator=(const memory_streambuf&) = delete;

protected:
    int_type underflow() override {
        return traits_type::eof();
    }

private:
    util::Buffer<char> buffer;
};

class memory_istream : public std::istream {
public:
    explicit memory_istream(util::Buffer<char> buffer)
        : std::istream(&buf), buf(std::move(buffer)) {}

private:
    memory_streambuf buf;
};
