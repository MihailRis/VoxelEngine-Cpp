#pragma once

#include <algorithm>
#include <string>
#include <vector>

namespace util {
    template <class T>
    inline bool contains(const std::vector<T>& vec, const T& value) {
        return std::find(vec.begin(), vec.end(), value) != vec.end();
    }

    template <class T>
    inline void concat(std::vector<T>& a, const std::vector<T>& b) {
        a.reserve(a.size() + b.size());
        a.insert(a.end(), b.begin(), b.end());
    }

    std::string to_string(const std::vector<std::string>& vec);
}
