#pragma once

#include "commons.hpp"
#include "data/dv.hpp"

template <typename CharT>
class BasicParser {
    using StringT = std::basic_string<CharT>;
    using StringViewT = std::basic_string_view<CharT>;

    void skipWhitespaceHashComment(bool newline = true);
protected:
    std::string_view filename;
    StringViewT source;
    uint pos = 0;
    uint line = 1;
    uint linestart = 0;
    bool hashComment = false;

    void skipWhitespace(bool newline = true);
    void skip(size_t n);
    void skipLine();
    void skipEmptyLines();
    bool skipTo(const StringT& substring);
    void expect(CharT expected);
    void expect(const StringT& substring);
    bool isNext(const StringT& substring);
    void expectNewLine();
    void goBack(size_t count = 1);
    void reset();

    int64_t parseSimpleInt(int base);
    dv::value parseNumber(int sign);
    dv::value parseNumber();
    StringT parseString(CharT chr, bool closeRequired = true);

    parsing_error error(const std::string& message);
public:
    StringViewT readUntil(CharT c);
    StringViewT readUntil(StringViewT s, bool nothrow);
    StringViewT readUntilWhitespace();
    StringViewT readUntilEOL();
    StringT parseName();
    StringT parseXmlName();
    bool hasNext();
    size_t remain() const;
    CharT peek();
    CharT peekInLine();
    CharT peekNoJump();
    CharT nextChar();

    BasicParser(std::string_view file, StringViewT source)
        : filename(file), source(source) {
    }
};

#include "BasicParser.inl"
