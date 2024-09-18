#include <gtest/gtest.h>

#include "util/Buffer.hpp"
#include "coders/binary_json.hpp"

TEST(BJSON, EncodeDecode) {
    const std::string name = "JSON-encoder";
    const int bytesSize = 5000;
    const int year = 2019;
    const float score = 3.141592;
    dv::objects::Bytes srcBytes(bytesSize);
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

        bjsonBytes = json::to_binary(object, false);
    }
    {
        auto object = json::from_binary(bjsonBytes.data(), bjsonBytes.size());
        
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
