#ifndef UTIL_LISTUTIL_HPP_
#define UTIL_LISTUTIL_HPP_

#include <algorithm>
#include <vector>
#include <string>
#include <queue>

namespace util {
    template<class T>
    bool contains(const std::vector<T>& vec, const T& value) {
        return std::find(vec.begin(), vec.end(), value) != vec.end();
    }

    std::string to_string(const std::vector<std::string>& vec);
}

#endif // UTIL_LISTUTIL_HPP_
