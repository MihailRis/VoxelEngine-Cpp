#pragma once

#define ZLIB_CONST
#include <zlib.h>
#include <istream>
#include <memory>
#include <array>
#include <stdexcept>

class deflate_istreambuf : public std::streambuf {
public:
    explicit deflate_istreambuf(std::istream& src) : src(src) {
        zstream.zalloc = Z_NULL;
        zstream.zfree = Z_NULL;
        zstream.opaque = Z_NULL;
        zstream.avail_in = 0;
        zstream.next_in = Z_NULL;
        
        int ret = inflateInit2(&zstream, -15);
        if (ret != Z_OK) {
            throw std::runtime_error("zlib init failed");
        }
    }

    ~deflate_istreambuf() {
        inflateEnd(&zstream);
    }

    deflate_istreambuf(const deflate_istreambuf&) = delete;
    deflate_istreambuf& operator=(const deflate_istreambuf&) = delete;

protected:
    int_type underflow() override {
        if (gptr() < egptr()) {
            return traits_type::to_int_type(*gptr());
        }

        if (eof) {
            return traits_type::eof();
        }

        zstream.next_out = reinterpret_cast<Bytef*>(outBuf.data());
        zstream.avail_out = outBuf.size();

        do {
            if (zstream.avail_in == 0) {
                src.read(inBuf.data(), inBuf.size());
                zstream.avail_in = static_cast<uInt>(src.gcount());
                zstream.next_in = reinterpret_cast<Bytef*>(inBuf.data());
                
                if (src.bad()) {
                    return traits_type::eof();
                }
            }

            int ret = inflate(&zstream, Z_NO_FLUSH);
            if (ret == Z_STREAM_END) {
                eof = true;
            } else if (ret != Z_OK) {
                if (ret == Z_BUF_ERROR && zstream.avail_out == outBuf.size()) {
                    continue;
                }
                return traits_type::eof();
            }

            const auto decompressed = outBuf.size() - zstream.avail_out;
            if (decompressed > 0) {
                setg(outBuf.data(), 
                        outBuf.data(), 
                        outBuf.data() + decompressed);
                return traits_type::to_int_type(*gptr());
            }

            if (eof) {
                return traits_type::eof();
            }
        } while (zstream.avail_in > 0 || !src.eof());

        return traits_type::eof();
    }
private:
    static constexpr size_t BUFFER_SIZE = 16384;

    std::istream& src;
    z_stream zstream {};
    std::array<char, BUFFER_SIZE> inBuf {};
    std::array<char, BUFFER_SIZE> outBuf {};
    bool eof = false;
};

class deflate_istream : public std::istream {
public:
    explicit deflate_istream(std::unique_ptr<std::istream> src)
        : std::istream(&buffer), source(std::move(src)), buffer(*source) {}

private:
    std::unique_ptr<std::istream> source;
    deflate_istreambuf buffer;
};
