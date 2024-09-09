#include <gtest/gtest.h>

#include "util/AreaMap2D.hpp"

TEST(AreaMap2D, BaseTest) {
    util::AreaMap2D<int> window({6, 6});
}
