#ifndef CODERS_COMMONS_H_
#define CODERS_COMMONS_H_

#include <string>
#include <stdexcept>
#include "../typedefs.h"

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
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_' || c == '-' || c == '.';
}

inline bool is_identifier_part(int c) {
    return is_identifier_start(c) || is_digit(c);
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

extern std::string escape_string(std::string s);

class parsing_error : public std::runtime_error {
public:
    std::string filename;
    std::string source;
    uint pos;
    uint line;
    uint linestart;

    parsing_error(std::string message, 
                    std::string filename, 
                    std::string source, 
                    uint pos, 
                    uint line, 
                    uint linestart);

    std::string errorLog() const;
};

class BasicParser {
protected:
    std::string filename;
    std::string source;
    uint pos = 0;
    uint line = 1;
    uint linestart = 0;

    virtual void skipWhitespace();
    void skip(size_t n);
    void skipLine();
    bool skipTo(const std::string& substring);
    void expect(char expected);
    void expect(const std::string& substring);
    char peek();
    char nextChar();
    bool hasNext();
    bool isNext(const std::string& substring);
    void expectNewLine();
    void goBack();

    std::string parseName();
    int64_t parseSimpleInt(int base);
    bool parseNumber(int sign, number_u& out);
    std::string parseString(char chr, bool closeRequired=true);

    parsing_error error(std::string message);

    BasicParser(std::string filename, std::string source);
};

#endif // CODERS_COMMONS_H_
