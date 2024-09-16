#pragma once

#include <string>

#include "data/dynamic.hpp"
#include "data/dv.hpp"
#include "typedefs.hpp"
#include "binary_json.hpp"

namespace json {
    dynamic::Map_sptr parse(std::string_view filename, std::string_view source);
    dynamic::Map_sptr parse(std::string_view source);

    dv::value parseDV(std::string_view filename, std::string_view source);
    dv::value parseDV(std::string_view source);

    std::string stringify(
        const dynamic::Map* obj, bool nice, const std::string& indent
    );

    std::string stringify(
        const dynamic::List* arr, bool nice, const std::string& indent
    );

    std::string stringify(
        const dynamic::Value& value, bool nice, const std::string& indent
    );

    std::string stringifyDV(
        const dv::value& value, bool nice, const std::string& indent="  "
    );
}
