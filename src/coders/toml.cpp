#include "toml.hpp"

#include "commons.hpp"
#include "../data/setting.hpp"
#include "../data/dynamic.hpp"
#include "../util/stringutil.hpp"
#include "../files/settings_io.hpp"

#include <math.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <assert.h>

using namespace toml;

class Reader : public BasicParser {
    SettingsHandler& handler;

    void skipWhitespace() override {
        BasicParser::skipWhitespace();
        if (hasNext() && source[pos] == '#') {
            skipLine();
            if (hasNext() && is_whitespace(peek())) {
                skipWhitespace();
            }
        }
    }
    void readSection(const std::string& section) {
        while (hasNext()) {
            skipWhitespace();
            if (!hasNext()) {
                break;
            }
            char c = nextChar();
            if (c == '[') {
                std::string name = parseName();
                pos++;
                readSection(name);
                return;
            }
            pos--;
            std::string name = section+"."+parseName();
            expect('=');
            c = peek();
            if (is_digit(c)) {
                auto num = parseNumber(1);
                if (handler.has(name)) {
                    handler.setValue(name, num);
                }
            } else if (c == '-' || c == '+') {
                int sign = c == '-' ? -1 : 1;
                pos++;
                auto num = parseNumber(sign);
                if (handler.has(name)) {
                    handler.setValue(name, num);
                }
            } else if (is_identifier_start(c)) {
                std::string identifier = parseName();
                if (handler.has(name)) {
                    if (identifier == "true" || identifier == "false") {
                        bool flag = identifier == "true";
                        handler.setValue(name, flag);
                    } else if (identifier == "inf") {
                        handler.setValue(name, INFINITY);
                    } else if (identifier == "nan") {
                        handler.setValue(name, NAN);
                    }
                }
            } else if (c == '"' || c == '\'') {
                pos++;
                std::string str = parseString(c);
                if (handler.has(name)) {
                    handler.setValue(name, str);
                }
            } else {
                throw error("feature is not supported");
            }
            expectNewLine();
        }
    }

public:
    Reader(
        SettingsHandler& handler,
        const std::string& file, 
        const std::string& source) 
    : BasicParser(file, source), handler(handler) {
    }

    void read() {
        skipWhitespace();
        if (!hasNext()) {
            return;
        }
        readSection("");
    }
};

void toml::parse(
    SettingsHandler& handler, 
    const std::string& file, 
    const std::string& source
) {
    Reader reader(handler, file, source);
    reader.read();
}

std::string toml::stringify(SettingsHandler& handler) {
    auto& sections = handler.getSections();

    std::stringstream ss;
    for (auto& section : sections) {
        ss << "[" << section.name << "]\n";
        for (const std::string& key : section.keys) {
            ss << key << " = ";
            auto setting = handler.getSetting(section.name+"."+key);
            assert(setting != nullptr);
            if (auto integer = dynamic_cast<IntegerSetting*>(setting)) {
                ss << integer->get();
            } else if (auto number = dynamic_cast<NumberSetting*>(setting)) {
                ss << number->get();
            } else if (auto flag = dynamic_cast<FlagSetting*>(setting)) {
                ss << (flag->get() ? "true" : "false");
            } else if (auto string = dynamic_cast<StringSetting*>(setting)) {
                ss << util::escape(string->get());
            }
            ss << "\n";
        }
        ss << "\n";
    }
    return ss.str();
}
