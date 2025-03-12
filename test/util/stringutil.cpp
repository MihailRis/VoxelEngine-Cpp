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

TEST(stringutil, base64) {
    srand(2019);
    for (size_t size = 0; size < 30; size++) {
        auto bytes = std::make_unique<ubyte[]>(size);
        for (int i = 0; i < size; i++) {
            bytes[i] = rand();
        }
        auto base64 = util::base64_encode(bytes.get(), size);
        auto decoded = util::base64_decode(base64);
        ASSERT_EQ(size, decoded.size());
        for (size_t i = 0; i < size; i++) {
            ASSERT_EQ(bytes[i], decoded[i]);
        }
    }
}

TEST(stringutil, base64_urlsafe) {
    srand(2019);
    for (size_t size = 0; size < 30; size++) {
        auto bytes = std::make_unique<ubyte[]>(size);
        for (int i = 0; i < size; i++) {
            bytes[i] = rand();
        }
        auto base64 = util::base64_urlsafe_encode(bytes.get(), size);
        auto decoded = util::base64_urlsafe_decode(base64);
        ASSERT_EQ(size, decoded.size());
        for (size_t i = 0; i < size; i++) {
            ASSERT_EQ(bytes[i], decoded[i]);
        }
    }
}
