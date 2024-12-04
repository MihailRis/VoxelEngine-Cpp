#pragma once

#include <string>
#include <memory>

struct FontStylesScheme;

namespace devtools {
    std::unique_ptr<FontStylesScheme> syntax_highlight(
        const std::string& lang, std::string_view source
    );
}
