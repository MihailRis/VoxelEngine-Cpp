#include <gtest/gtest.h>

#include "coders/vec3.hpp"
#include "io/io.hpp"

TEST(VEC3, Decode) {
    auto file = std::filesystem::u8path(
        "res/models/block.vec3"
    );
    auto bytes = io::read_bytes_buffer(file);
    auto model = vec3::load(file.u8string(), bytes);
}
