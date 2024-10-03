#include <gtest/gtest.h>

#include "coders/byte_utils.hpp"

TEST(byte_utils, BytesBuildAndRead) {
    ByteBuilder builder;
    builder.putCStr("MAGIC.#");
    builder.put(50);
    builder.putInt16(1980);
    builder.putInt32(123456789);
    builder.putInt64(98765432123456789LL);
    auto data = builder.build();

    ByteReader reader(data.data(), data.size());
    reader.checkMagic("MAGIC.#", 8);
    EXPECT_EQ(reader.get(), 50);
    EXPECT_EQ(reader.getInt16(), 1980);
    EXPECT_EQ(reader.getInt32(), 123456789);
    EXPECT_EQ(reader.getInt64(), 98765432123456789LL);
}
