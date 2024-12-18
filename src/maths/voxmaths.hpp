#pragma once

inline constexpr int floordiv(int a, int b) {
    if (a < 0 && a % b) {
        return (a / b) - 1;
    }
    return a / b;
}

inline constexpr bool is_pot(int a) {
    return (a > 0) && ((a & (a - 1)) == 0);
}

inline constexpr unsigned floorlog2(unsigned x) {
    return x == 1 ? 0 : 1 + floorlog2(x >> 1);
}

template<int b>
inline constexpr int floordiv(int a) {
    if constexpr (is_pot(b)) {
        return a >> floorlog2(b);
    } else {
        return floordiv(a, b);
    }
}

inline constexpr int ceildiv(int a, int b) {
    if (a > 0 && a % b) {
        return a / b + 1;
    }
    return a / b;
}
