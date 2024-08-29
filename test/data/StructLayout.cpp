#include "data/StructLayout.hpp"

#include <gtest/gtest.h>

using namespace data;

TEST(StructLayout, ReadWrite) {
    ubyte buffer[16] {};

    std::vector<Field> fields {
        Field {FieldType::I8, "a", 1},
        Field {FieldType::CHAR, "s", 4},
        Field {FieldType::F32, "f", 1},
    };
    auto layout = StructLayout::create(fields);
    EXPECT_EQ(layout.size(), 9);

    layout.setInteger(buffer, 42, "a");
    EXPECT_EQ(layout.getInteger(buffer, "a"), 42);

    layout.setNumber(buffer, 3.141592f, "f");
    EXPECT_FLOAT_EQ(layout.getNumber(buffer, "f"), 3.141592f);

    layout.setChars(buffer, "hello", "s");
    EXPECT_EQ(layout.getChars(buffer, "s"), "hell");
}

TEST(StructLayout, Unicode) {
    ubyte buffer[8] {};
    std::vector<Field> fields {
        Field {FieldType::CHAR, "text", 5},
    };
     auto layout = StructLayout::create(fields);
     EXPECT_EQ(layout.size(), 5);

     layout.setUnicode(buffer, u8"テキストデモ", "text");
     EXPECT_EQ(layout.getChars(buffer, "text"), std::string(u8"テ"));

     layout.setUnicode(buffer, u8"пример", "text");
     EXPECT_EQ(layout.getChars(buffer, "text"), std::string(u8"пр"));
}
