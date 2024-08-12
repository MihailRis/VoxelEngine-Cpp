#include <gtest/gtest.h>

#include "typedefs.hpp"
#include "coders/rle.hpp"

TEST(RLE, EncodeDecode) {
    const int initial_size = 50'000;
    uint8_t initial[initial_size];
    uint8_t next = rand();
    for (int i = 0; i < initial_size; i++) {
        initial[i] = next;
        if (rand() % 13 == 0) {
            next = rand();
        }
    }
    uint8_t encoded[initial_size * 2];
    auto encoded_size = rle::encode(initial, initial_size, encoded);
    uint8_t decoded[initial_size * 2];
    auto decoded_size = rle::decode(encoded, encoded_size, decoded);
    
    EXPECT_EQ(decoded_size, initial_size);

    for (int i = 0; i < decoded_size; i++) {
        EXPECT_EQ(decoded[i], initial[i]);
    }
}

TEST(ExtRLE, EncodeDecode) {
    const int initial_size = 50'000;
    uint8_t initial[initial_size];
    uint8_t next = rand();
    for (int i = 0; i < initial_size; i++) {
        initial[i] = next;
        if (rand() % 13 == 0) {
            next = rand();
        }
    }
    uint8_t encoded[initial_size * 2];
    auto encoded_size = extrle::encode(initial, initial_size, encoded);
    uint8_t decoded[initial_size * 2];
    auto decoded_size = extrle::decode(encoded, encoded_size, decoded);
    
    EXPECT_EQ(decoded_size, initial_size);

    for (int i = 0; i < decoded_size; i++) {
        EXPECT_EQ(decoded[i], initial[i]);
    }
}
