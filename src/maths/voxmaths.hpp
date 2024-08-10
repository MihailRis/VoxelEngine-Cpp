#pragma once

#include "typedefs.hpp"

inline constexpr int floordiv(int a, int b) {
    if (a < 0 && a % b) {
        return (a / b) - 1;
    }
    return a / b;
}

inline constexpr int ceildiv(int a, int b) {
    if (a > 0 && a % b) {
        return a / b + 1;
    }
    return a / b;
}
