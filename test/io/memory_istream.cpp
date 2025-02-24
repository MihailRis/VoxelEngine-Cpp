#include <gtest/gtest.h>

#include "io/memory_istream.hpp"

TEST(io, memory_istream) {
    const char data[] = "Hello, world!";
    const int n = std::strlen(data);

    util::Buffer<char> buffer(data, n);
    memory_istream stream(std::move(buffer));

    ASSERT_TRUE(stream.good());

    std::string text(n, '\0');
    stream.read(text.data(), n);
    ASSERT_EQ(text, std::string(data));
    stream.read(text.data(), 1);
    ASSERT_TRUE(stream.eof());
}
