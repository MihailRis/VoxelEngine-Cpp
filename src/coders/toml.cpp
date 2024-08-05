#include "toml.hpp"

#include <assert.h>
#include <math.h>

#include <iomanip>
#include <sstream>

#include <data/dynamic.hpp>
#include <data/setting.hpp>
#include <files/settings_io.hpp>
#include <util/stringutil.hpp>
#include "commons.hpp"

using namespace toml;

class TomlReader : BasicParser {
    dynamic::Map_sptr root;

    void skipWhitespace() override {
        BasicParser::skipWhitespace();
        if (hasNext() && source[pos] == '#') {
            skipLine();
            if (hasNext() && is_whitespace(peek())) {
                skipWhitespace();
            }
        }
    }

    dynamic::Map& getSection(const std::string& section) {
        if (section.empty()) {
            return *root;
        }
        size_t offset = 0;
        auto& rootMap = *root;
        do {
            size_t index = section.find('.', offset);
            if (index == std::string::npos) {
                auto map = rootMap.map(section);
                if (map == nullptr) {
                    return rootMap.putMap(section);
                }
                return *map;
            }
            auto subsection = section.substr(offset, index);
            auto map = rootMap.map(subsection);
            if (map == nullptr) {
                rootMap = rootMap.putMap(subsection);
            } else {
                rootMap = *map;
            }
            offset = index + 1;
        } while (true);
    }

    void readSection(const std::string& section, dynamic::Map& map) {
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
                map.put(name, parseNumber(1));
            } else if (c == '-' || c == '+') {
                int sign = c == '-' ? -1 : 1;
                pos++;
                map.put(name, parseNumber(sign));
            } else if (is_identifier_start(c)) {
                std::string identifier = parseName();
                if (identifier == "true" || identifier == "false") {
                    map.put(name, identifier == "true");
                } else if (identifier == "inf") {
                    map.put(name, INFINITY);
                } else if (identifier == "nan") {
                    map.put(name, NAN);
                }
            } else if (c == '"' || c == '\'') {
                pos++;
                map.put(name, parseString(c));
            } else {
                throw error("feature is not supported");
            }
            expectNewLine();
        }
    }
public:
    TomlReader(std::string_view file, std::string_view source)
        : BasicParser(file, source) {
        root = dynamic::create_map();
    }

    dynamic::Map_sptr read() {
        skipWhitespace();
        if (!hasNext()) {
            return root;
        }
        readSection("", *root);
        return root;
    }
};

dynamic::Map_sptr toml::parse(std::string_view file, std::string_view source) {
    return TomlReader(file, source).read();
}

void toml::parse(
    SettingsHandler& handler, std::string_view file, std::string_view source
) {
    auto map = parse(file, source);
    for (auto& entry : map->values) {
        const auto& sectionName = entry.first;
        auto sectionMap = std::get_if<dynamic::Map_sptr>(&entry.second);
        if (sectionMap == nullptr) {
            continue;
        }
        for (auto& sectionEntry : (*sectionMap)->values) {
            const auto& name = sectionEntry.first;
            auto& value = sectionEntry.second;
            auto fullname = sectionName + "." + name;
            if (handler.has(fullname)) {
                handler.setValue(fullname, value);
            }
        }
    }
}

std::string toml::stringify(dynamic::Map& root, const std::string& name) {
    std::stringstream ss;
    if (!name.empty()) {
        ss << "[" << name << "]\n";
    }
    for (auto& entry : root.values) {
        if (!std::holds_alternative<dynamic::Map_sptr>(entry.second)) {
            ss << entry.first << " = ";
            ss << entry.second << "\n";
        }
    }
    for (auto& entry : root.values) {
        if (auto submap = std::get_if<dynamic::Map_sptr>(&entry.second)) {
            ss << "\n"
               << toml::stringify(
                      **submap,
                      name.empty() ? entry.first : name + "." + entry.first
                  );
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
