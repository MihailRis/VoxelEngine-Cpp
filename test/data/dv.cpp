#include <gtest/gtest.h>

#include "data/dv.hpp"

TEST(dv, dv) {
    auto value = dv::object();
    {
        auto& list = value.list("elements");
        for (int i = 0; i < 1; i++) {
            auto& obj = list.object();
            obj["name"] = "user";
            obj["age"] = 90;
            auto& position = obj.list("position");
            position.add(40);
            position.add(-41);
            position.add(52);
        }
    }
    {
        const auto& list = value["elements"];
        for (const auto& obj : list) {
            EXPECT_EQ(obj["name"].asString(), "user");
            EXPECT_EQ(obj["age"].asInteger(), 90);
            auto& position = obj["position"];
            EXPECT_EQ(position[0].asInteger(), 40);
            EXPECT_EQ(position[1].asInteger(), -41);
            EXPECT_EQ(position[2].asInteger(), 52);
        }
    }
}
