#pragma once

#include <glm/glm.hpp>

namespace util {
    template<class T>
    struct pow {
        constexpr T operator()(T a, T b) const {
            return glm::pow(a, b);
        }
    };

    template<class T>
    struct min {
        constexpr T operator()(T a, T b) const {
            return glm::min(a, b);
        }
    };

    template<class T>
    struct max {
        constexpr T operator()(T a, T b) const {
            return glm::max(a, b);
        }
    };

    template<class T>
    struct abs {
        constexpr T operator()(T a) const {
            return glm::abs(a);
        }
    };
}
