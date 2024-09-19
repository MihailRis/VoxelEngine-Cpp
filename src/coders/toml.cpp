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
            c = peek();
            if (is_digit(c)) {
                map[name] = parseNumber(1);
            } else if (c == '-' || c == '+') {
                int sign = c == '-' ? -1 : 1;
                pos++;
                map[name] = parseNumber(sign);
            } else if (is_identifier_start(c)) {
                std::string identifier = parseName();
                if (identifier == "true" || identifier == "false") {
                    map[name] = identifier == "true";
                } else if (identifier == "inf") {
                    map[name] = INFINITY;
                } else if (identifier == "nan") {
                    map[name] = NAN;
                }
            } else if (c == '"' || c == '\'') {
                pos++;
                map[name] = parseString(c);
            } else {
                throw error("feature is not supported");
            }
            expectNewLine();
        }
    }
public:
    TomlReader(std::string_view file, std::string_view source)
        : BasicParser(file, source) {
        root = dv::object();
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
