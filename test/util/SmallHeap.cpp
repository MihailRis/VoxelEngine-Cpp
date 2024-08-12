#include <gtest/gtest.h>

#include "util/SmallHeap.hpp"

using namespace util;

TEST(SmallHeap, Allocation) {
    auto index = 0;
    auto size = 4;

    SmallHeap<uint16_t, uint8_t> map;
    auto ptr = map.allocate(index, size);
    EXPECT_EQ(map.sizeOf(ptr), size);
    EXPECT_EQ(ptr, map.find(index));
}

TEST(SmallHeap, MultipleAllocation) {
    SmallHeap<uint16_t, uint8_t> map;
    map.allocate(32, 4);
    map.allocate(1, 5);
    EXPECT_EQ(map.sizeOf(map.find(32)), 4);
    EXPECT_EQ(map.sizeOf(map.find(1)), 5);
}

TEST(SmallHeap, Free) {
    SmallHeap<uint16_t, uint8_t> map;
    map.free(map.allocate(5, 4));
    EXPECT_EQ(map.find(5), nullptr);
}

TEST(SmallHeap, ReAllocationSame) {
    SmallHeap<uint16_t, uint8_t> map;
    auto ptr1 = map.allocate(1, 2);
    auto ptr2 = map.allocate(1, 2);
    EXPECT_EQ(ptr1, ptr2);
}

TEST(SmallHeap, ReAllocationDifferent) {
    SmallHeap<uint16_t, uint8_t> map;
    map.allocate(1, 34);
    map.allocate(1, 2);
    EXPECT_EQ(map.sizeOf(map.find(1)), 2);
}

TEST(SmallHeap, RandomFill) {
    SmallHeap<uint16_t, uint8_t> map;
    int n = 3'000;
    map.allocate(n, 123);
    for (int i = 0; i < n; i++) {
        int index = rand() % n;
        int size = rand() % 254 + 1;
        map.allocate(index, size);
    }
    EXPECT_EQ(map.sizeOf(map.find(n)), 123);
}
