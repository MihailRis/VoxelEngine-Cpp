#include <gtest/gtest.h>

#include "data/dynamic.hpp"
#include "coders/binary_json.hpp"

TEST(BJSON, EncodeDecode) {
    const std::string name = "JSON-encoder";
    const int bytesSize = 5000;
    const int year = 2019;
    const float score = 3.141592;
    dynamic::ByteBuffer srcBytes(bytesSize);
    for (int i = 0; i < bytesSize; i ++) {
        srcBytes[i] = rand();
    }

    std::vector<ubyte> bjsonBytes;
    {
        dynamic::Map map;
        map.put("name", name);
        map.put("year", year);
        map.put("score", score);
        map.put("data", &srcBytes);

        bjsonBytes = json::to_binary(&map, false);
    }
    {
        auto map = json::from_binary(bjsonBytes.data(), bjsonBytes.size());
        EXPECT_EQ(map->get<std::string>("name"), name);
        EXPECT_EQ(map->get<integer_t>("year"), year);
        EXPECT_FLOAT_EQ(map->get<number_t>("score"), score);
        auto bytesptr = map->bytes("data");
        const auto& bytes = *bytesptr;
        EXPECT_EQ(bytes.size(), bytesSize);
        for (int i = 0; i < bytesSize; i++) {
            EXPECT_EQ(bytes[i], srcBytes[i]);
        }
    }
}

TEST(BJSON, EncodeDecodeDV) {
    const std::string name = "JSON-encoder";
    const int bytesSize = 5000;
    const int year = 2019;
    const float score = 3.141592;
    dynamic::ByteBuffer srcBytes(bytesSize);
    for (int i = 0; i < bytesSize; i ++) {
        srcBytes[i] = rand();
    }

    std::vector<ubyte> bjsonBytes;
    {
        auto object = dv::object();
        object["name"] = name;
        object["year"] = year;
        object["score"] = score;
        object["data"] = srcBytes;

        bjsonBytes = json::to_binaryDV(object, false);
    }
    {
        auto object = json::from_binaryDV(bjsonBytes.data(), bjsonBytes.size());
        
        EXPECT_EQ(object["name"].asString(), name);
        EXPECT_EQ(object["year"].asInteger(), year);
        EXPECT_FLOAT_EQ(object["score"].asNumber(), score);
        const auto& bytes = object["data"].asBytes();
        EXPECT_EQ(bytes.size(), bytesSize);
        for (int i = 0; i < bytesSize; i++) {
            EXPECT_EQ(bytes[i], srcBytes[i]);
        }
    }
}
