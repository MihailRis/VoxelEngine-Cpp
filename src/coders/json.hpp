#pragma once

#include <string>

#include "data/dv.hpp"
#include "typedefs.hpp"
#include "binary_json.hpp"

namespace json {
    dv::value parse(std::string_view filename, std::string_view source);
    dv::value parse(std::string_view source);

    std::string stringify(
        const dv::value& value,
        bool nice,
        const std::string& indent = "  ",
        bool escapeUtf8 = false
    );
}
