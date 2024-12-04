#include "syntax_highlighting.hpp"

#include "coders/commons.hpp"
#include "coders/lua_parsing.hpp"
#include "graphics/core/Font.hpp"

using namespace devtools;

static std::unique_ptr<FontStylesScheme> build_styles(
    const std::vector<lua::Token>& tokens
) {
    FontStylesScheme styles {
        {
            {false, false, glm::vec4(0.8f, 0.8f, 0.8f, 1)}, // default
            {true, false, glm::vec4(0.9, 0.6f, 0.4f, 1)},   // keyword
            {false, false, glm::vec4(0.4, 0.8f, 0.5f, 1)},  // string
            {false, false, glm::vec4(0.3, 0.3f, 0.3f, 1)},  // comment
            {false, false, glm::vec4(0.4, 0.45f, 0.5f, 1)}, // self
            {true, false, glm::vec4(1.0f, 0.2f, 0.1f, 1)}, // unexpected
        }, 
        {}
    };
    size_t offset = 0;
    for (int i = 0; i < tokens.size(); i++) {
        const auto& token = tokens.at(i);
        if (token.tag != lua::TokenTag::KEYWORD &&
            token.tag != lua::TokenTag::STRING &&
            token.tag != lua::TokenTag::INTEGER &&
            token.tag != lua::TokenTag::NUMBER &&
            token.tag != lua::TokenTag::COMMENT &&
            token.tag != lua::TokenTag::UNEXPECTED) {
            continue;
        }
        if (token.start.pos > offset) {
            int n = token.start.pos - offset;
            styles.map.insert(styles.map.end(), token.start.pos - offset, 0);
        }
        offset = token.end.pos;
        int styleIndex;
        switch (token.tag) {
            case lua::TokenTag::KEYWORD: styleIndex = 1; break;
            case lua::TokenTag::STRING:
            case lua::TokenTag::INTEGER:
            case lua::TokenTag::NUMBER: styleIndex = 2; break;
            case lua::TokenTag::COMMENT: styleIndex = 3; break;
            case lua::TokenTag::UNEXPECTED: styleIndex = 5; break;
            default:
                styleIndex = 0;
                break;
        }
        styles.map.insert(
            styles.map.end(), token.end.pos - token.start.pos, styleIndex
        );
    }
    styles.map.push_back(0);
    return std::make_unique<FontStylesScheme>(std::move(styles));
}

std::unique_ptr<FontStylesScheme> devtools::syntax_highlight(
    const std::string& lang, std::string_view source
) {
    try {
        if (lang == "lua") {
            auto tokens = lua::tokenize("<string>", source);
            return build_styles(tokens);
        } else {
            return nullptr;
        }
    } catch (const parsing_error& err) {
        return nullptr;
    }
}
