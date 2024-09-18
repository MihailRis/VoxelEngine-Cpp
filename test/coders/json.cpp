#include <gtest/gtest.h>

#include "coders/json.hpp"
#include "util/stringutil.hpp"

TEST(JSON, EncodeDecode) {
    const std::string name = "JSON-encoder";
    const int bytesSize = 20;
    const int year = 2019;
    const float score = 3.141592;
    dv::objects::Bytes srcBytes(bytesSize);
    for (int i = 0; i < bytesSize; i ++) {
        srcBytes[i] = rand();
    }

    std::string text;
    {
        auto object = dv::object();
        object["name"] = name;
        object["year"] = year;
        object["score"] = score;
        object["data"] = srcBytes;

        text = json::stringify(object, false, "");
    }
    {
        auto object = json::parse(text);
        EXPECT_EQ(object["name"].asString(), name);
        EXPECT_EQ(object["year"].asInteger(), year);
        EXPECT_FLOAT_EQ(object["score"].asNumber(), score);
        auto b64string = object["data"].asString();

        auto bytes = util::base64_decode(b64string);
        EXPECT_EQ(bytes.size(), bytesSize);
        for (int i = 0; i < bytesSize; i++) {
            EXPECT_EQ(bytes[i], srcBytes[i]);
        }
    }
}
