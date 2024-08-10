#include "commons.hpp"

#include <math.h>

#include <sstream>
#include <stdexcept>

#include "util/stringutil.hpp"

inline double power(double base, int64_t power) {
    double result = 1.0;
    for (int64_t i = 0; i < power; i++) {
        result *= base;
    }
    return result;
}

parsing_error::parsing_error(
    const std::string& message,
    std::string_view filename,
    std::string_view source,
    uint pos,
    uint line,
    uint linestart
)
    : std::runtime_error(message),
      filename(filename),
      pos(pos),
      line(line),
      linestart(linestart) {
    size_t end = source.find("\n", linestart);
    if (end == std::string::npos) {
        end = source.length();
    }
    this->source = source.substr(linestart, end - linestart);
}

std::string parsing_error::errorLog() const {
    std::stringstream ss;
    uint linepos = pos - linestart;
    ss << "parsing error in file '" << filename;
    ss << "' at " << (line + 1) << ":" << linepos << ": " << this->what()
       << "\n";
    ss << source << "\n";
    for (uint i = 0; i < linepos; i++) {
        ss << " ";
    }
    ss << "^";
    return ss.str();
}

BasicParser::BasicParser(std::string_view file, std::string_view source)
    : filename(file), source(source) {
}

void BasicParser::skipWhitespace() {
    while (hasNext()) {
        char next = source[pos];
        if (next == '\n') {
            line++;
            linestart = ++pos;
            continue;
        }
        if (is_whitespace(next)) {
            pos++;
        } else {
            break;
        }
    }
}

void BasicParser::skip(size_t n) {
    n = std::min(n, source.length() - pos);

    for (size_t i = 0; i < n; i++) {
        char next = source[pos++];
        if (next == '\n') {
            line++;
            linestart = pos;
        }
    }
}

void BasicParser::skipLine() {
    while (hasNext()) {
        if (source[pos] == '\n') {
            pos++;
            linestart = pos;
            line++;
            break;
        }
        pos++;
    }
}

bool BasicParser::skipTo(const std::string& substring) {
    size_t idx = source.find(substring, pos);
    if (idx == std::string::npos) {
        skip(source.length() - pos);
        return false;
    } else {
        skip(idx - pos);
        return true;
    }
}

bool BasicParser::hasNext() {
    return pos < source.length();
}

bool BasicParser::isNext(const std::string& substring) {
    if (source.length() - pos < substring.length()) {
        return false;
    }
    return source.substr(pos, substring.length()) == substring;
}

char BasicParser::nextChar() {
    if (!hasNext()) {
        throw error("unexpected end");
    }
    return source[pos++];
}

void BasicParser::expect(char expected) {
    char c = peek();
    if (c != expected) {
        throw error("'" + std::string({expected}) + "' expected");
    }
    pos++;
}

void BasicParser::expect(const std::string& substring) {
    if (substring.empty()) return;
    for (uint i = 0; i < substring.length(); i++) {
        if (source.length() <= pos + i || source[pos + i] != substring[i]) {
            throw error(util::quote(substring) + " expected");
        }
    }
    pos += substring.length();
}

void BasicParser::expectNewLine() {
    while (hasNext()) {
        char next = source[pos];
        if (next == '\n') {
            line++;
            linestart = ++pos;
            return;
        }
        if (is_whitespace(next)) {
            pos++;
        } else {
            throw error("line separator expected");
        }
    }
}

void BasicParser::goBack(size_t count) {
    if (pos < count) {
        throw std::runtime_error("pos < jump");
    }
    if (pos) {
        pos -= count;
    }
}

void BasicParser::reset() {
    pos = 0;
}

char BasicParser::peekInLine() {
    while (hasNext()) {
        char next = source[pos];
        if (next == '\n') {
            return next;
        }
        if (is_whitespace(next)) {
            pos++;
        } else {
            break;
        }
    }
    if (pos >= source.length()) {
        throw error("unexpected end");
    }
    return source[pos];
}

char BasicParser::peek() {
    skipWhitespace();
    if (pos >= source.length()) {
        throw error("unexpected end");
    }
    return source[pos];
}

char BasicParser::peekNoJump() {
    if (pos >= source.length()) {
        throw error("unexpected end");
    }
    return source[pos];
}

std::string_view BasicParser::readUntil(char c) {
    int start = pos;
    while (hasNext() && source[pos] != c) {
        pos++;
    }
    return source.substr(start, pos - start);
}

std::string_view BasicParser::readUntilEOL() {
    int start = pos;
    while (hasNext() && source[pos] != '\r' && source[pos] != '\n') {
        pos++;
    }
    return source.substr(start, pos - start);
}

std::string BasicParser::parseName() {
    char c = peek();
    if (!is_identifier_start(c)) {
        if (c == '"') {
            pos++;
            return parseString(c);
        }
        throw error("identifier expected");
    }
    int start = pos;
    while (hasNext() && is_identifier_part(source[pos])) {
        pos++;
    }
    return std::string(source.substr(start, pos - start));
}

int64_t BasicParser::parseSimpleInt(int base) {
    char c = peek();
    int index = hexchar2int(c);
    if (index == -1 || index >= base) {
        throw error("invalid number literal");
    }
    int64_t value = index;
    pos++;
    while (hasNext()) {
        c = source[pos];
        while (c == '_') {
            c = source[++pos];
        }
        index = hexchar2int(c);
        if (index == -1 || index >= base) {
            return value;
        }
        value *= base;
        value += index;
        pos++;
    }
    return value;
}

dynamic::Value BasicParser::parseNumber() {
    switch (peek()) {
        case '-':
            skip(1);
            return parseNumber(-1);
        case '+':
            skip(1);
            return parseNumber(1);
        default:
            return parseNumber(1);
    }
}

dynamic::Value BasicParser::parseNumber(int sign) {
    char c = peek();
    int base = 10;
    if (c == '0' && pos + 1 < source.length() &&
        (base = is_box(source[pos + 1])) != 10) {
        pos += 2;
        return parseSimpleInt(base);
    } else if (c == 'i' && pos + 2 < source.length() && source[pos + 1] == 'n' && source[pos + 2] == 'f') {
        pos += 3;
        return INFINITY * sign;
    } else if (c == 'n' && pos + 2 < source.length() && source[pos + 1] == 'a' && source[pos + 2] == 'n') {
        pos += 3;
        return NAN * sign;
    }
    int64_t value = parseSimpleInt(base);
    if (!hasNext()) {
        return value * sign;
    }
    c = source[pos];
    if (c == 'e' || c == 'E') {
        pos++;
        int s = 1;
        if (peek() == '-') {
            s = -1;
            pos++;
        } else if (peek() == '+') {
            pos++;
        }
        return sign * value * power(10.0, s * parseSimpleInt(10));
    }
    if (c == '.') {
        pos++;
        int64_t expo = 1;
        while (hasNext() && source[pos] == '0') {
            expo *= 10;
            pos++;
        }
        int64_t afterdot = 0;
        if (hasNext() && is_digit(source[pos])) {
            afterdot = parseSimpleInt(10);
        }
        expo *= power(10, fmax(0, log10(afterdot) + 1));
        c = source[pos];

        double dvalue = (value + (afterdot / (double)expo));
        if (c == 'e' || c == 'E') {
            pos++;
            int s = 1;
            if (peek() == '-') {
                s = -1;
                pos++;
            } else if (peek() == '+') {
                pos++;
            }
            return sign * dvalue * power(10.0, s * parseSimpleInt(10));
        }
        return sign * dvalue;
    }
    return sign * value;
}

std::string BasicParser::parseString(char quote, bool closeRequired) {
    std::stringstream ss;
    while (hasNext()) {
        char c = source[pos];
        if (c == quote) {
            pos++;
            return ss.str();
        }
        if (c == '\\') {
            pos++;
            c = nextChar();
            if (c >= '0' && c <= '7') {
                pos--;
                ss << (char)parseSimpleInt(8);
                continue;
            }
            switch (c) {
                case 'n':
                    ss << '\n';
                    break;
                case 'r':
                    ss << '\r';
                    break;
                case 'b':
                    ss << '\b';
                    break;
                case 't':
                    ss << '\t';
                    break;
                case 'f':
                    ss << '\f';
                    break;
                case '\'':
                    ss << '\\';
                    break;
                case '"':
                    ss << '"';
                    break;
                case '\\':
                    ss << '\\';
                    break;
                case '/':
                    ss << '/';
                    break;
                case '\n':
                    pos++;
                    continue;
                default:
                    throw error(
                        "'\\" + std::string({c}) + "' is an illegal escape"
                    );
            }
            continue;
        }
        if (c == '\n' && closeRequired) {
            throw error("non-closed string literal");
        }
        ss << c;
        pos++;
    }
    if (closeRequired) {
        throw error("unexpected end");
    }
    return ss.str();
}

parsing_error BasicParser::error(const std::string& message) {
    return parsing_error(message, filename, source, pos, line, linestart);
}
