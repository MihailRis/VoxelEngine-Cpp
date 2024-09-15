#include <gtest/gtest.h>

#include "data/dv.hpp"
#include "data/dynamic.hpp"
#include "util/timeutil.hpp"

TEST(dv, dv) {
    // speed comparsion with data/dynamic

    {
        timeutil::ScopeLogTimer log(444);
        auto map = dynamic::create_map();
        auto& list = map->putList("elements");
        for (int i = 0; i < 10000; i++) {
            auto& obj = list.putMap();
            obj.put("name", "user");
            obj.put("age", 90);
            auto pos = dynamic::create_list({40, -41, 52});
            obj.put("position", pos);
        }
    }
    {
        timeutil::ScopeLogTimer log(555);
        auto value = dv::object();
        auto& list = value.list("elements");
        for (int i = 0; i < 10000; i++) {
            auto& obj = list.object();
            obj["name"] = "user";
            obj["age"] = 90;
            obj["position"] = dv::list({40, -41, 52});
        }
    }
}
