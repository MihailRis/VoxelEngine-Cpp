#include <gtest/gtest.h>

#include "data/dv.hpp"

TEST(dv, dv) {
    auto value = dv::object();
    {
        auto& list = value.list("elements");
        for (int i = 0; i < 10; i++) {
            auto& obj = list.object();
            obj["name"] = "user";
            obj["age"] = 90;
            obj["confirmed"] = true;
            obj["position"] = dv::list({40, -41, 52});
        }
    }
    {
        const auto& list = value["elements"];
        for (const auto& obj : list) {
            EXPECT_EQ(obj["name"].asString(), "user");
            EXPECT_EQ(obj["age"].asInteger(), 90);
            EXPECT_EQ(obj["confirmed"].asBoolean(), true);
            auto& position = obj["position"];
            EXPECT_EQ(position[0].asInteger(), 40);
            EXPECT_EQ(position[1].asInteger(), -41);
            EXPECT_EQ(position[2].asInteger(), 52);
        }
    }
}
