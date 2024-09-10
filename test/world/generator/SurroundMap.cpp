#include <gtest/gtest.h>
#include <atomic>

#include "world/generator/SurroundMap.hpp"

TEST(SurroundMap, InitTest) {
    int maxLevelZone = 50;
    int x = 20;
    int y = 30;
    int8_t maxLevel = 5;
    std::atomic_int affected = 0;

    SurroundMap map(maxLevelZone, maxLevel);
    map.setLevelCallback(1, [&affected](auto, auto) {
        affected++;
    });
    map.completeAt(x, y);
    EXPECT_EQ(affected, (maxLevel * 2 - 1)*(maxLevel * 2 - 1));

    for (int ly = -maxLevel+1; ly < maxLevel; ly++) {
        for (int lx = -maxLevel+1; lx < maxLevel; lx++) {
            int levelExpected = maxLevel-std::max(std::abs(lx), std::abs(ly));
            EXPECT_EQ(map.at(x+lx, y+ly), levelExpected);
        }
    }
}
