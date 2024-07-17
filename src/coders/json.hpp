#ifndef CODERS_JSON_HPP_
#define CODERS_JSON_HPP_

#include "binary_json.hpp"

#include "../data/dynamic.hpp"
#include "../typedefs.hpp"

#include <string>

namespace json {
    dynamic::Map_sptr parse(std::string_view filename, std::string_view source);
    dynamic::Map_sptr parse(std::string_view source);

    std::string stringify(
        const dynamic::Map* obj,
        bool nice,
        const std::string& indent
    );

    std::string stringify(
        const dynamic::Value& value,
        bool nice,
        const std::string& indent
    );
}

#endif // CODERS_JSON_HPP_
