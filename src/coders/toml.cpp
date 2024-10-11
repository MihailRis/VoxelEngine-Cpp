#include "toml.hpp"

#include <assert.h>
#include <math.h>

#include <iomanip>
#include <sstream>

#include "data/setting.hpp"
#include "files/settings_io.hpp"
#include "util/stringutil.hpp"
#include "commons.hpp"

using namespace toml;

class TomlReader : BasicParser {
    dv::value root;

    void skipWhitespace() override {
        BasicParser::skipWhitespace();
        if (hasNext() && source[pos] == '#') {
            skipLine();
            if (hasNext() && is_whitespace(peek())) {
                skipWhitespace();
            }
        }
    }

    dv::value& getSection(const std::string& section) {
        if (section.empty()) {
            return root;
        }
        size_t offset = 0;
        auto rootMap = &root;
        do {
            size_t index = section.find('.', offset);
            if (index == std::string::npos) {
                auto map = rootMap->at(section);
                if (!map) {
                    return rootMap->object(section);
                }
                return *map;
            }
            auto subsection = section.substr(offset, index);
            auto map = rootMap->at(subsection);
            if (!map) {
                rootMap = &rootMap->object(subsection);
            } else {
                rootMap = &*map;
            }
            offset = index + 1;
        } while (true);
    }

    dv::value parseValue() {
        char c = peek();
        if (is_digit(c)) {
            return parseNumber(1);
        } else if (c == '-' || c == '+') {
            int sign = c == '-' ? -1 : 1;
            pos++;
            // parse numeric literal
            auto value = parseNumber(sign);
            if (hasNext() && peekNoJump() == '-') {
                // parse timestamp // TODO: implement
                throw error("timestamps support is not implemented yet");
            }
            return value;
        } else if (is_identifier_start(c)) {
            // parse keywords
            std::string keyword = parseName();
            if (keyword == "true" || keyword == "false") {
                return keyword == "true";
            } else if (keyword == "inf") {
                return INFINITY;
            } else if (keyword == "nan") {
                return NAN;
            }
            throw error("unknown keyword " + util::quote(keyword));
        } else if (c == '"' || c == '\'') {
            pos++;
            return parseString(c);
        } else if (c == '[') {
            // parse array
            pos++;
            dv::list_t values;
            while (peek() != ']') {
                values.push_back(parseValue());
                if (peek() != ']') {
                    expect(',');
                }
            }
            pos++;
            return dv::value(std::move(values));
        } else if (c == '{') {
            // parse inline table
            pos++;
            auto table = dv::object();
            while (peek() != '}') {
                auto key = parseName();
                expect('=');
                table[key] = parseValue();
                if (peek() != '}') {
                    expect(',');
                }
            }
            pos++;
            return table;
        } else {
            throw error("feature is not supported");
        }
    }

    void readSection(const std::string& section, dv::value& map) {
        while (hasNext()) {
            skipWhitespace();
            if (!hasNext()) {
                break;
            }
            char c = nextChar();
            if (c == '[') {
                std::string name = parseName();
                pos++;
                readSection(name, getSection(name));
                return;
            }
            pos--;
            std::string name = parseName();
            expect('=');
            map[name] = parseValue();
            expectNewLine();
        }
    }
public:
    TomlReader(std::string_view file, std::string_view source)
        : BasicParser(file, source), root(dv::object()) {
    }

    dv::value read() {
        skipWhitespace();
        if (!hasNext()) {
            return std::move(root);
        }
        readSection("", root);
        return std::move(root);
    }
};

void toml::parse(
    SettingsHandler& handler, std::string_view file, std::string_view source
) {
    auto map = parse(file, source);
    for (const auto& [sectionName, sectionMap] : map.asObject()) {
        if (!sectionMap.isObject()) {
            continue;
        }
        for (const auto& [name, value] : sectionMap.asObject()) {
            auto fullname = sectionName + "." + name;
            if (handler.has(fullname)) {
                handler.setValue(fullname, value);
            }
        }
    }
}

dv::value toml::parse(std::string_view file, std::string_view source) {
    return TomlReader(file, source).read();
}

std::string toml::stringify(const dv::value& root, const std::string& name) {
    std::stringstream ss;
    if (!name.empty()) {
        ss << "[" << name << "]\n";
    }
    for (const auto& [key, value] : root.asObject()) {
        if (!value.isObject()) {
            ss << key << " = " << value << "\n";
        }
    }
    for (const auto& [key, value] : root.asObject()) {
        if (value.isObject()) {
            ss << "\n" << toml::stringify(value,
                      name.empty() ? key : name + "." + key);
        }
    }
    return ss.str();
}

std::string toml::stringify(SettingsHandler& handler) {
    auto& sections = handler.getSections();

    std::stringstream ss;
    for (auto& section : sections) {
        ss << "[" << section.name << "]\n";
        for (const std::string& key : section.keys) {
            ss << key << " = ";
            auto setting = handler.getSetting(section.name + "." + key);
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
