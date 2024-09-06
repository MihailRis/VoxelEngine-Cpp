#include "util/stringutil.hpp"

#include <gtest/gtest.h>

TEST(stringutil, crop_utf8) {
    // Project source files must be UTF-8 encoded
    std::string str = u8"пример";
    str = str.substr(0, util::crop_utf8(str, 7));
    EXPECT_EQ(str, u8"при");
}

TEST(stringutil, utf8) {
    std::string str = u8"テキストデモ";
    auto u32str = util::str2u32str_utf8(str);
    std::string str2 = util::u32str2str_utf8(u32str);
    EXPECT_EQ(str, str2);
}
