#include <gtest/gtest.h>

#include "world/generator/SurroundMap.hpp"

TEST(SurroundMap, InitTest) {
    int8_t maxLevel = 2;
    SurroundMap map(50, maxLevel);
    map.completeAt(25, 25);
    EXPECT_EQ(map.at(25, 25), maxLevel);
}
