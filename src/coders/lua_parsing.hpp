#pragma once

#include <string>
#include <vector>

namespace lua {
    struct Location {
        int pos;
        int lineStart;
        int line;
    };

    enum class TokenTag {
        KEYWORD, NAME, INTEGER, NUMBER, OPEN_BRACKET, CLOSE_BRACKET, STRING,
        OPERATOR, COMMA, SEMICOLON, UNEXPECTED
    };

    struct Token {
        TokenTag tag;
        std::string text;
        Location start;
        Location end;

        Token(TokenTag tag, std::string text, Location start, Location end)
            : tag(tag),
              text(std::move(text)),
              start(std::move(start)),
              end(std::move(end)) {
        }
    };

    bool is_lua_keyword(std::string_view view);

    std::vector<Token> tokenize(std::string_view file, std::string_view source);
}
