#include "lua_parsing.hpp"

#include <set>

#include "BasicParser.hpp"

using namespace lua;
using namespace devtools;

static std::set<std::wstring_view> keywords {
    L"and", L"break", L"do", L"else", L"elseif", L"end", L"false", L"for", L"function", 
    L"if", L"in", L"local", L"nil", L"not", L"or", L"repeat", L"return", L"then", L"true",
    L"until", L"while"
};

static bool is_lua_keyword(std::wstring_view view) {
    return keywords.find(view) != keywords.end();
}

inline bool is_lua_identifier_start(int c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_';
}

inline bool is_lua_identifier_part(int c) {
    return is_lua_identifier_start(c) || is_digit(c);
}

inline bool is_lua_operator_start(int c) {
    return c == '=' || c == '~' || c == '+' || c == '-' || c == '/' || c == '*'
        || c == '%' || c == '^' || c == '#' || c == '<' || c == '>' || c == ':'
        || c == '.';
}

class Tokenizer : BasicParser<wchar_t> {
    std::vector<Token> tokens;
public:
    Tokenizer(std::string_view file, std::wstring_view source)
        : BasicParser(file, source) {
    }

    std::wstring parseLuaName() {
        char c = peek();
        if (!is_identifier_start(c)) {
            throw error("identifier expected");
        }
        int start = pos;
        while (hasNext() && is_identifier_part(source[pos])) {
            pos++;
        }
        return std::wstring(source.substr(start, pos - start));
    }

    inline Location currentLocation() const {
        return Location {
            static_cast<int>(pos),
            static_cast<int>(linestart),
            static_cast<int>(line)};
    }

    void emitToken(
        TokenTag tag, std::wstring name, Location start, bool standalone=false
    ) {
        tokens.emplace_back(
            tag,
            std::move(name),
            std::move(start),
            currentLocation()
        );
        if (standalone) skip(1);
    }

    /// @brief Get next operator token without checking operator for existing
    std::wstring parseOperator() {
        int start = pos;
        wchar_t first = peek();
        switch (first) {
            case '#': case '+': case '/': case '*': case '^':
            case '%':
                skip(1);
                return std::wstring({first});
            case '-':
                skip(1);
                if (hasNext() && peekNoJump() == '-') {
                    skip(1);
                    return L"--";
                }
                return std::wstring({first});
        }
        skip(1);
        char second = peekNoJump();
        if ((first == '=' && second == '=') || (first == '~' && second == '=') ||
            (first == '<' && second == '=') || (first == '>' && second == '=')) {
            skip(1);
            return std::wstring(source.substr(start, pos - start));
        }
        if (first == '.' && second == '.') {
            skip(1);
            if (peekNoJump() == '.') {
                skip(1);
            }
        }
        return std::wstring(source.substr(start, pos - start));
    }

    std::vector<Token> tokenize() {
        skipWhitespace();
        while (hasNext()) {
            skipWhitespace();
            if (!hasNext()) {
                continue;
            }
            wchar_t c = peek();
            auto start = currentLocation();
            if (is_lua_identifier_start(c)) {
                auto name = parseLuaName();
                TokenTag tag = (is_lua_keyword(name) ? TokenTag::KEYWORD : TokenTag::NAME);
                emitToken(
                    tag,
                    std::move(name),
                    start
                );
                continue;
            } else if (is_digit(c)) {
                dv::value value;
                auto tag = TokenTag::UNEXPECTED;
                try {
                    value = parseNumber(1);
                    tag = value.isInteger() ? TokenTag::INTEGER
                                            : TokenTag::NUMBER;
                } catch (const parsing_error& err) {}

                auto literal = source.substr(start.pos, pos - start.pos);
                emitToken(tag, std::wstring(literal), start);
                continue;
            }
            switch (c) {
                case '(': case '[': case '{': 
                    if (isNext(L"[==[")) {
                        auto string = readUntil(L"]==]", true);
                        skip(4);
                        emitToken(
                            TokenTag::COMMENT,
                            std::wstring(string) + L"]==]",
                            start
                        );
                        continue;
                    } else if (isNext(L"[[")) {
                        skip(2);
                        auto string = readUntil(L"]]", true);
                        skip(2);
                        emitToken(TokenTag::STRING, std::wstring(string), start);
                        continue;
                    }
                    emitToken(TokenTag::OPEN_BRACKET, std::wstring({c}), start, true);
                    continue;
                case ')': case ']': case '}': 
                    emitToken(TokenTag::CLOSE_BRACKET, std::wstring({c}), start, true);
                    continue;
                case ',':
                    emitToken(TokenTag::COMMA, std::wstring({c}), start, true);
                    continue;
                case ';':
                    emitToken(TokenTag::SEMICOLON, std::wstring({c}), start, true);
                    continue;
                case '\'': case '"': {
                    skip(1);
                    auto string = parseString(c, false);
                    emitToken(TokenTag::STRING, std::move(string), start);
                    continue;
                }
                default: break;
            }
            if (is_lua_operator_start(c)) {
                auto text = parseOperator();
                if (text == L"--") {
                    auto string = readUntilEOL();
                    emitToken(TokenTag::COMMENT, std::wstring(string), start);
                    skipLine();
                    continue;
                }
                emitToken(TokenTag::OPERATOR, std::move(text), start);
                continue;
            }
            auto text = readUntilWhitespace();
            emitToken(TokenTag::UNEXPECTED, std::wstring(text), start);
        }
        return std::move(tokens);
    }
};

std::vector<Token> lua::tokenize(std::string_view file, std::wstring_view source) {
    return Tokenizer(file, source).tokenize();
}
