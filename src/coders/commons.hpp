#ifndef CODERS_COMMONS_HPP_
#define CODERS_COMMONS_HPP_

#include "../data/dynamic.hpp"
#include "../typedefs.hpp"

#include <string>
#include <stdexcept>

inline int is_box(int c) {
    switch (c) {
        case 'B':
        case 'b':
            return 2;
        case 'O':
        case 'o':
            return 8;
        case 'X':
        case 'x':
            return 16; 
    }
    return 10;
}

inline bool is_digit(int c) {
    return (c >= '0' && c <= '9');
}

inline bool is_whitespace(int c) {
    return c == ' ' || c == '\n' || c == '\r' || c == '\t' || c == '\f';
}

inline bool is_identifier_start(int c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_' || c == '.';
}

inline bool is_identifier_part(int c) {
    return is_identifier_start(c) || is_digit(c) || c == '-';
}

inline int hexchar2int(int c) {
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

class parsing_error : public std::runtime_error {
public:
    std::string filename;
    std::string source;
    uint pos;
    uint line;
    uint linestart;

    parsing_error(
        const std::string& message,
        std::string_view filename, 
        std::string_view source, 
        uint pos, 
        uint line, 
        uint linestart
    );
    std::string errorLog() const;
};

class BasicParser {
protected:
    std::string_view filename;
    std::string_view source;
    uint pos = 0;
    uint line = 1;
    uint linestart = 0;

    virtual void skipWhitespace();
    void skip(size_t n);
    void skipLine();
    bool skipTo(const std::string& substring);
    void expect(char expected);
    void expect(const std::string& substring);
    bool isNext(const std::string& substring);
    void expectNewLine();
    void goBack(size_t count=1);

    int64_t parseSimpleInt(int base);
    dynamic::Value parseNumber(int sign);
    std::string parseString(char chr, bool closeRequired=true);

    parsing_error error(const std::string& message);

public:
    std::string_view readUntil(char c);
    std::string parseName();
    bool hasNext();
    char peek();
    char peekNoJump();
    char nextChar();

    BasicParser(std::string_view file, std::string_view source);
};

#endif // CODERS_COMMONS_HPP_
