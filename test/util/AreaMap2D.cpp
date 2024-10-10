#include <gtest/gtest.h>
#include <atomic>

#include "util/AreaMap2D.hpp"

TEST(AreaMap2D, BaseTest) {
    util::AreaMap2D<int> window({7, 5});
    window.setCenter(0, 0);
    {
        int i = 1;
        for (int y = -2; y <= 2; y++) {
            for (int x = -3; x <= 3; x++, i++) {
                window.set(x, y, i);
            }
        }
    }
    EXPECT_EQ(window.count(), 7 * 5);
    {
        int i = 1;
        for (int y = -2; y <= 2; y++) {
            for (int x = -3; x <= 3; x++, i++) {
                EXPECT_EQ(window.require(x, y), i);
            }
        }
    }
    window.set(0, 0, 0);
    EXPECT_EQ(window.count(), 7 * 5 - 1);
}


TEST(AreaMap2D, ResizeTest) {
    util::AreaMap2D<int> window({7, 5});
    window.setCenter(0, 0);
    {
        int i = 1;
        for (int y = -2; y <= 2; y++) {
            for (int x = -3; x <= 3; x++, i++) {
                window.set(x, y, i);
            }
        }
    }
    EXPECT_EQ(window.count(), 7 * 5);
    window.resize(9, 7);
    window.setCenter(0, 0);
    EXPECT_EQ(window.count(), 7 * 5);
    window.resize(7, 5);

    EXPECT_EQ(window.count(), 7 * 5);
    {
        int i = 1;
        for (int y = -2; y <= 2; y++) {
            for (int x = -3; x <= 3; x++, i++) {
                EXPECT_EQ(window.require(x, y), i);
            }
        }
    }
}

TEST(AreaMap2D, TranslateWithOut) {
    util::AreaMap2D<int> window({7, 5});
    window.setCenter(0, 0);
    {
        int i = 1;
        for (int y = -2; y <= 2; y++) {
            for (int x = -3; x <= 3; x++, i++) {
                window.set(x, y, i);
            }
        }
    }
    std::atomic_int outside = 0;
    window.setOutCallback([&outside](auto, auto, auto) {
        outside++;
    });
    window.setCenter(-2, -1);
    EXPECT_EQ(window.require(-3, -2), 1);
    EXPECT_EQ(outside, 15);
    EXPECT_EQ(window.count(), 20);
}
