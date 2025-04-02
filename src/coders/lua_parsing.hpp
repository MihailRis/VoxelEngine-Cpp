#pragma once

#include <string>
#include <vector>

#include "devtools/syntax.hpp"

namespace lua {
    std::vector<devtools::Token> tokenize(
        std::string_view file, std::wstring_view source
    );
}
