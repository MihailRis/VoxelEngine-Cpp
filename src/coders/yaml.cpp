#include "yaml.hpp"
#include "BasicParser.hpp"

using namespace yaml;

namespace {
    enum Chomping {
        CLIP, STRIP, KEEP
    };

    class Parser : BasicParser<char> {
    public:
        Parser(std::string_view filename, std::string_view source);

        dv::value parseValue();
        dv::value parseFullValue(int indent);
        dv::value parseArray(int indent = 0);
        dv::value parseObject(dv::value&& object, int indent = 0);
        dv::value parseInlineArray();
        dv::value parseInlineObject();
    private:
        int countIndent();
        bool expectIndent(int indent);
        std::string_view readYamlIdentifier();
        std::string readMultilineString(int indent, bool eols, Chomping chomp);
    };
}

inline bool is_yaml_identifier_char(int c) {
    return c > 20 && c != ':' && c != ' ' && c != '\n' && c != '\r' &&
           c != '\t' && c != '\f' && c != '\v';
}

static dv::value perform_literal(std::string_view literal) {
    if (literal == "true" || literal == "True" || 
        literal == "false" || literal == "False") {
        return literal[0] == 't';
    }
    if (literal == "null" || literal == "Null") {
        return nullptr;
    }
    return std::string(literal);
}

Parser::Parser(std::string_view filename, std::string_view source)
    : BasicParser(filename, source) {
    hashComment = true;
}

bool Parser::expectIndent(int required) {
    int indent = 0;
    while (hasNext() && source[pos] == ' ' && indent < required) {
        indent++;
        pos++;
    }
    return indent >= required;
}

std::string Parser::readMultilineString(int indent, bool eols, Chomping chomp) {
    int next_indent = countIndent();
    if (next_indent <= indent) {
        throw error("indentation error");
    }
    std::stringstream ss;
    ss << readUntilEOL();
    if (hasNext()) {
        skip(1);
    }
    int trailingEmpties = 0;
    while (true) {
        while (expectIndent(next_indent)) {
            trailingEmpties = 0;
            ss << (eols ? '\n' : ' ');
            ss << readUntilEOL();
            if (hasNext()) {
                skip(1);
            }
        }
        while (true) {
            skipWhitespace(false);
            if (!hasNext() || source[pos] != '\n') {
                break;
            }
            skip(1);
            trailingEmpties++;
        }
        if (!expectIndent(next_indent)) {
            break;
        }
        pos = linestart;
    }
    if (chomp == KEEP) {
        for (int i = 0; i < trailingEmpties - 1; i++) {
            ss << (eols ? '\n' : ' ');
        }
    }
    ss << '\n';

    pos = linestart;

    auto string = ss.str();
    if (chomp == STRIP) {
        util::trim(string);
    }
    return string;
}

std::string_view Parser::readYamlIdentifier() {
    char c = peek();
    if (!is_yaml_identifier_char(c)) {
        throw error("identifier expected");
    }
    int start = pos;
    while (hasNext() && is_yaml_identifier_char(source[pos])) {
        pos++;
    }
    return source.substr(start, pos - start);
}

int Parser::countIndent() {
    int indent = 0;
    while (hasNext() && source[pos] == ' ') {
        indent++;
        pos++;
    }
    return indent;
}

dv::value Parser::parseValue() {
    char c = peek();
    if (is_digit(c)) {
        return parseNumber(1);
    } else if (c == '-' || c == '+') {
        skip(1);
        return parseNumber(c == '-' ? -1 : 1);
    } else if (c == '"' || c == '\'') {
        skip(1);
        return parseString(c, true);
    } else if (c == '[') {
        return parseInlineArray();
    } else if (c == '{') {
        return parseInlineObject();
    } else {
        return perform_literal(readUntilEOL());
    }
    throw error("unexpected character");
}

dv::value Parser::parseInlineArray() {
    expect('[');
    auto list = dv::list();
    while (peek() != ']') {
        if (peek() == '#') {
            skipLine();
            continue;
        }
        list.add(parseValue());

        char next = peek();
        if (next == ',') {
            pos++;
        } else if (next == ']') {
            break;
        } else {
            throw error("',' expected");
        }
    }
    pos++;
    return list;
}

dv::value Parser::parseInlineObject() {
    expect('{');
    dv::value object = dv::object();
    while (peek() != '}') {
        if (peek() == '#') {
            skipLine();
            continue;
        }
        auto name = readYamlIdentifier();
        expect(':');
        object[std::string(name)] = parseValue();

        char next = peek();
        if (next == ',') {
            pos++;
        } else if (next == '}') {
            break;
        } else {
            throw error("',' expected");
        }
    }
    pos++;
    return object;
}

dv::value Parser::parseFullValue(int indent) {
    dv::value value;
    char c = source[pos];
    if (c == '\n') {
        skip(1);
        skipEmptyLines();
        int init_pos = pos;
        int next_indent = countIndent();
        if (next_indent < indent) {
            throw error("indentation error");
        }
        if (source[pos] == '-') {
            pos = init_pos;
            return parseArray(next_indent);
        } else {
            pos = init_pos;
            return parseObject(dv::object(), next_indent);
        }
    } else if (is_digit(c)) {
        return parseNumber(1);
    } else if (c == '-' || c == '+') {
        skip(1);
        return parseNumber(c == '-' ? -1 : 1);
    } else if (c == '"' || c == '\'') {
        skip(1);
        return parseString(c, true);
    } else if (c == '[') {
        return parseInlineArray();
    } else if (c == '{') {
        return parseInlineObject();
    } else if (c == '|' || c == '>') {
        skip(1);
        Chomping chomp = CLIP;
        if (source[pos] == '-' || source[pos] == '+') {
            chomp = source[pos] == '-' ? STRIP : KEEP;
            skip(1);
        }
        skipWhitespace(false);
        expectNewLine();
        return readMultilineString(indent, c == '|', chomp);
    } else {
        return perform_literal(readUntilEOL());
    }
}

dv::value Parser::parseArray(int indent) {
    dv::value list = dv::list();

    while (hasNext()) {
        skipEmptyLines();
        int next_indent = countIndent();
        if (next_indent < indent) {
            pos = linestart;
            break;
        }
        expect('-');
        skipWhitespace();
        size_t nlpos = source.find('\n', pos);
        size_t colonpos = source.find(':', pos);
        if (nlpos == std::string::npos && colonpos == std::string::npos) {
            list.add(perform_literal(readUntilEOL()));
            break;
        }
        if (nlpos < colonpos) {
            list.add(parseFullValue(next_indent));
            skipLine();
        } else {
            auto name = readYamlIdentifier();
            expect(':');
            skipWhitespace(false);
            dv::value object = dv::object();
            object[std::string(name)] = parseFullValue(next_indent);
            skipEmptyLines();
            next_indent = countIndent();
            if (next_indent > indent) {
                pos = linestart;
                object = parseObject(std::move(object), next_indent);
            } else {
                pos = linestart;
            }
            list.add(std::move(object));
        }
    }
    return list;
}

dv::value Parser::parseObject(dv::value&& object, int indent) {
    skipEmptyLines();
    while (hasNext()) {
        size_t prev_pos = pos;
        int next_indent = countIndent();
        if (source[pos] == '\n') {
            skip(1);
            continue;
        }
        if (next_indent < indent) {
            pos = prev_pos;
            break;
        }
        char c = peek();
        if (!is_yaml_identifier_char(c)) {
            if (!is_whitespace(c)) {
                throw error("invalid character");
            }
            continue;
        }
        auto name = readYamlIdentifier();
        expect(':');
        skipWhitespace(false);
        object[std::string(name)] = parseFullValue(indent);
        skipEmptyLines();
    }
    return object;
}

dv::value yaml::parse(std::string_view filename, std::string_view source) {
    return Parser(filename, source).parseObject(dv::object());
}

dv::value yaml::parse(std::string_view source) {
    return parse("[string]", source);
}
