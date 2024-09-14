#include <gtest/gtest.h>

#include "data/dv.hpp"

TEST(dv, dv) {
    auto value = dv::object({
        {"d", "test"}
    });
    auto list = dv::list({
        561, 52, "hello"
    });
    list.add("world");
    value["a"] = 51;
}
