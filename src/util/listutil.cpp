#include "listutil.h"
#include "../util/stringutil.h"

#include <sstream>

std::string util::to_string(const std::vector<std::string>& vec) {
    std::stringstream ss;
    ss << "[";
    for (size_t i = 0; i < vec.size(); i++) {
        ss << util::quote(vec.at(i));
        if (i < vec.size()-1) {
            ss << ", ";
        }
    }
    ss << "]";
    return ss.str();
}
