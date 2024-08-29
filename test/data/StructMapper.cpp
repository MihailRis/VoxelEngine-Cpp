#include "data/StructMapper.hpp"

#include <gtest/gtest.h>

using namespace data;

TEST(StructMapper, ReadWrite) {
    ubyte buffer[16] {};

    std::vector<Field> fields {
        Field {FieldType::I8, "a", 1},
        Field {FieldType::I32, "b", 1},
        Field {FieldType::F32, "f", 1},
    };
    auto mapping = StructMapping::create(fields);
    EXPECT_EQ(mapping.size(), 9);

    mapping.setInteger(buffer, 42, "a");
    EXPECT_EQ(mapping.getInteger(buffer, "a"), 42);

    mapping.setNumber(buffer, 3.141592f, "f");
    EXPECT_FLOAT_EQ(mapping.getNumber(buffer, "f"), 3.141592f);
}
