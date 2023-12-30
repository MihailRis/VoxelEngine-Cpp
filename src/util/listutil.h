#ifndef UTIL_LISTUTIL_H_
#define UTIL_LISTUTIL_H_

#include <vector>

namespace util {
    template<class T>
    bool contains(std::vector<T> vec, const T& value) {
        return std::find(vec.begin(), vec.end(), value) != vec.end();
    }
}

#endif // UTIL_LISTUTIL_H_
