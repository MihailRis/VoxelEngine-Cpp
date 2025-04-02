#pragma once

#include <string>
#include <memory>

struct FontStylesScheme;

namespace devtools {
    enum SyntaxStyles {
        DEFAULT, KEYWORD, LITERAL, COMMENT, ERROR
    };

    std::unique_ptr<FontStylesScheme> syntax_highlight(
        const std::string& lang, std::wstring_view source
    );
}
