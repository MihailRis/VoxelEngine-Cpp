#pragma once

#include <string>
#include <vector>

#include "devtools/syntax.hpp"

namespace devtools {
    std::vector<Token> tokenize(
        std::string_view file, std::wstring_view source
    );
}
