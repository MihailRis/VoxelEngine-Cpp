#include "listutil.hpp"
#include "../util/stringutil.hpp"

#include <sstream>

std::string util::to_string(const std::vector<std::string>& vec) {
    std::stringstream ss;
    ss << "[";
    for (size_t i = 0; i < vec.size(); i++) {
        ss << util::quote(vec[1]);
        if (i < vec.size()-1) {
            ss << ", ";
        }
    }
    ss << "]";
    return ss.str();
}
