#include "syntax_highlighting.hpp"

#include "coders/commons.hpp"
#include "coders/syntax_parser.hpp"
#include "graphics/core/Font.hpp"

using namespace devtools;

static std::unique_ptr<FontStylesScheme> build_styles(
    const std::vector<devtools::Token>& tokens
) {
    using devtools::TokenTag;
    FontStylesScheme styles {
        {
            {false, false, false, false, glm::vec4(0.8f, 0.8f, 0.8f, 1)}, // default
            {true, false, false, false, glm::vec4(0.9, 0.6f, 0.4f, 1)},   // keyword
            {false, false, false, false, glm::vec4(0.4, 0.8f, 0.5f, 1)},  // string
            {false, false, false, false, glm::vec4(0.3, 0.3f, 0.3f, 1)},  // comment
            {true, false, false, false, glm::vec4(1.0f, 0.2f, 0.1f, 1)},  // unexpected
        }, 
        {}
    };
    size_t offset = 0;
    for (int i = 0; i < tokens.size(); i++) {
        const auto& token = tokens.at(i);
        if (token.tag != TokenTag::KEYWORD &&
            token.tag != TokenTag::STRING &&
            token.tag != TokenTag::INTEGER &&
            token.tag != TokenTag::NUMBER &&
            token.tag != TokenTag::COMMENT &&
            token.tag != TokenTag::UNEXPECTED) {
            continue;
        }
        if (token.start.pos > offset) {
            styles.map.insert(styles.map.end(), token.start.pos - offset, 0);
        }
        offset = token.end.pos;
        int styleIndex;
        switch (token.tag) {
            case TokenTag::KEYWORD: styleIndex = SyntaxStyles::KEYWORD; break;
            case TokenTag::STRING:
            case TokenTag::INTEGER:
            case TokenTag::NUMBER: styleIndex = SyntaxStyles::LITERAL; break;
            case TokenTag::COMMENT: styleIndex = SyntaxStyles::COMMENT; break;
            case TokenTag::UNEXPECTED: styleIndex = SyntaxStyles::ERROR; break;
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
    const std::string& lang, std::wstring_view source
) {
    try {
        if (lang == "lua") {
            auto tokens = tokenize("<string>", source);
            return build_styles(tokens);
        } else {
            return nullptr;
        }
    } catch (const parsing_error& err) {
        return nullptr;
    }
}
