#include <gtest/gtest.h>

#include "world/generator/SurroundMap.hpp"

TEST(SurroundMap, InitTest) {
    int w = 8;
    int h = 8;

    SurroundMap map;
    map.createEntry({w/2, h/2});
    map.createEntry({w/2+1, h/2+1});
    EXPECT_EQ(map.upgrade().size(), 12);
}
