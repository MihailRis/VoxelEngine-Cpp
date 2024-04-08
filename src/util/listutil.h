#ifndef UTIL_LISTUTIL_H_
#define UTIL_LISTUTIL_H_

#include <algorithm>
#include <vector>
#include <queue>

namespace util {
    template<class T>
    bool contains(const std::vector<T>& vec, const T& value) {
        return std::find(vec.begin(), vec.end(), value) != vec.end();
    }

    template<class T>
    bool contains(const std::queue<T>& queue, const T& value) {
        return std::find(queue.begin(), queue.end(), value) != queue.end();
    }
}

#endif // UTIL_LISTUTIL_H_
