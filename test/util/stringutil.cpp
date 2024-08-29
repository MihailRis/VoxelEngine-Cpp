#include "util/stringutil.hpp"

#include <gtest/gtest.h>

TEST(stringutil, crop_utf8) {
    // Project source files must be UTF-8 encoded
    std::string str = u8"пример";
    str = str.substr(0, util::crop_utf8(str, 7));
    EXPECT_EQ(str, u8"при");
}
