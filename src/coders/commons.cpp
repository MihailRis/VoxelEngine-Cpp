#include "commons.h"

#include <sstream>
#include <math.h>

using std::string;

inline int char2int(int c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'a' && c <= 'f') {
        return 10 + c - 'a';
    }
    if (c >= 'A' && c <= 'F') {
        return 10 + c - 'A';
    }
    return -1;
}

inline double power(double base, int64_t power) {
    double result = 1.0;
    for (int64_t i = 0; i < power; i++) {
        result *= base;
    }
    return result;
}

parsing_error::parsing_error(string message, 
                string filename, 
                string source, 
                uint pos, 
                uint line, 
                uint linestart)
    : std::runtime_error(message), filename(filename), source(source), 
      pos(pos), line(line), linestart(linestart) {
}

string parsing_error::errorLog() const {
    std::stringstream ss;
    uint linepos = pos - linestart;
    ss << "parsing error in file '" << filename;
    ss << "' at " << (line+1) << ":" << linepos << ": " << this->what() << "\n";
    size_t end = source.find("\n", linestart);
    if (end == string::npos) {
        end = source.length();
    }
    ss << source.substr(linestart, end-linestart) << "\n";
    for (uint i = 0; i < linepos; i++) {
        ss << " ";
    }
    ss << "^";
    return ss.str();

}

string escape_string(string s) {
    std::stringstream ss;
    ss << '"';
    for (char c : s) {
        switch (c) {
            case '\n': ss << "\\n"; break;
            case '\r': ss << "\\r"; break;
            case '\t': ss << "\\t"; break;
            case '\f': ss << "\\f"; break;
            case '\b': ss << "\\b"; break;
            case '"': ss << "\\\""; break;
            case '\\': ss << "\\\\"; break;
            default:
                if (c < ' ') {
                    ss << "\\" << std::oct << (int)c;
                    break;
                }
                ss << c;
                break;
        }
    }
    ss << '"';
    return ss.str();
}

BasicParser::BasicParser(std::string file, std::string source) : filename(file), source(source) {
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

bool BasicParser::hasNext() {
    return pos < source.length();
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
        throw error("'"+string({expected})+"' expected");
    }
    pos++;
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

char BasicParser::peek() {
    skipWhitespace();
    if (pos >= source.length()) {
        throw error("unexpected end");
    }
    return source[pos];
}

string BasicParser::parseName() {
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
    return source.substr(start, pos-start);
}

int64_t BasicParser::parseSimpleInt(int base) {
    char c = peek();
    int index = char2int(c);
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
        index = char2int(c);
        if (index == -1 || index >= base) {
            return value;
        }
        value *= base;
        value += index;
        pos++;
    }
    return value;
}

bool BasicParser::parseNumber(int sign, number_u& out) {
    char c = peek();
    int base = 10;
    if (c == '0' && pos + 1 < source.length() && 
          (base = is_box(source[pos+1])) != 10) {
        pos += 2;
        out.ival = parseSimpleInt(base);
        return true;
    } else if (c == 'i' && pos + 2 < source.length() && source[pos+1] == 'n' && source[pos+2] == 'f') {
        pos += 3;
        out.fval = INFINITY * sign;
        return false;
    } else if (c == 'n' && pos + 2 < source.length() && source[pos+1] == 'a' && source[pos+2] == 'n') {
        pos += 3;
        out.fval = NAN * sign;
        return false;
    }
    int64_t value = parseSimpleInt(base);
    if (!hasNext()) {
        out.ival = value * sign;
        return true;
    }
    c = source[pos];
    if (c == 'e' || c == 'E') {
        pos++;
        int s = 1;
        if (peek() == '-') {
            s = -1;
            pos++;
        } else if (peek() == '+'){
            pos++;
        }
        out.fval = sign * value * power(10.0, s * parseSimpleInt(10));
        return false;
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
            } else if (peek() == '+'){
                pos++;
            }
            out.fval = sign * dvalue * power(10.0, s * parseSimpleInt(10));
            return false;
        }
        out.fval = sign * dvalue;
        return false;
    }
    out.ival = sign * value;
    return true;
}

string BasicParser::parseString(char quote) {
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
                case 'n': ss << '\n'; break;
                case 'r': ss << '\r'; break;
                case 'b': ss << '\b'; break;
                case 't': ss << '\t'; break;
                case 'f': ss << '\f'; break;
                case '\'': ss << '\\'; break;
                case '"': ss << '"'; break;
                case '\\': ss << '\\'; break;
                case '/': ss << '/'; break;
                case '\n': pos++; continue;
                default:
                    throw error("'\\" + string({c}) + "' is an illegal escape");
            }
            continue;
        }
        if (c == '\n') {
            throw error("non-closed string literal");
        }
        ss << c;
        pos++;
    }
    throw error("unexpected end");
}

parsing_error BasicParser::error(std::string message) {
    return parsing_error(message, filename, source, pos, line, linestart);
}