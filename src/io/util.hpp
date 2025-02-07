#pragma once

#include <string>
#include <stdexcept>

namespace io {
    inline bool is_valid_name(std::string_view name) {
        static std::string illegalChars = "\\/%?!<>:; ";
        for (char c : illegalChars) {
            if (name.find(c) != std::string::npos) {
                return false;
            }
        }
        return !name.empty();
    }
}
