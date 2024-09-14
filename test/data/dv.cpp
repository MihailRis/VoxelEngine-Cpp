#include <gtest/gtest.h>

#include "data/dv.hpp"

TEST(dv, dv) {
    dv::value value = dv::object({
        {"d", "test"}
    });
    value["a"] = 51;
}
