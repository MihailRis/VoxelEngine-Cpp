#include <gtest/gtest.h>

#include "voxels/Block.hpp"

TEST(Test1, Test1) {
    Block block("test");
    EXPECT_STREQ(block.name.c_str(), "test");
}
