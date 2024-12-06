#pragma once

#include <string>
#include <vector>

#include "devtools/syntax.hpp"

namespace lua {
    bool is_lua_keyword(std::string_view view);

    std::vector<devtools::Token> tokenize(
        std::string_view file, std::string_view source
    );
}
