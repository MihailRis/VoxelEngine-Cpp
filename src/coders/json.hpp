#ifndef CODERS_JSON_HPP_
#define CODERS_JSON_HPP_

#include "commons.hpp"
#include "binary_json.hpp"

#include "../data/dynamic.hpp"
#include "../typedefs.hpp"

#include <vector>
#include <string>
#include <stdint.h>
#include <stdexcept>
#include <unordered_map>

namespace json {
    std::unique_ptr<dynamic::Map> parse(const std::string& filename, const std::string& source);
    std::unique_ptr<dynamic::Map> parse(const std::string& source);

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
