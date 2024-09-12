#include "json.hpp"

#include <math.h>

#include <iomanip>
#include <memory>
#include <sstream>

#include "data/dynamic.hpp"
#include "util/stringutil.hpp"
#include "commons.hpp"

using namespace json;
using namespace dynamic;

class Parser : BasicParser {
    std::unique_ptr<dynamic::List> parseList();
    std::unique_ptr<dynamic::Map> parseObject();
    dynamic::Value parseValue();
public:
    Parser(std::string_view filename, std::string_view source);

    std::unique_ptr<dynamic::Map> parse();
};

inline void newline(
    std::stringstream& ss, bool nice, uint indent, const std::string& indentstr
) {
    if (nice) {
        ss << "\n";
        for (uint i = 0; i < indent; i++) {
            ss << indentstr;
        }
    } else {
        ss << ' ';
    }
}

void stringifyObj(
    const Map* obj,
    std::stringstream& ss,
    int indent,
    const std::string& indentstr,
    bool nice
);

void stringifyArr(
    const List* list,
    std::stringstream& ss,
    int indent,
    const std::string& indentstr,
    bool nice
);

void stringifyValue(
    const Value& value,
    std::stringstream& ss,
    int indent,
    const std::string& indentstr,
    bool nice
) {
    if (auto map = std::get_if<Map_sptr>(&value)) {
        stringifyObj(map->get(), ss, indent, indentstr, nice);
    } else if (auto listptr = std::get_if<List_sptr>(&value)) {
        stringifyArr(listptr->get(), ss, indent, indentstr, nice);
    } else if (auto flag = std::get_if<bool>(&value)) {
        ss << (*flag ? "true" : "false");
    } else if (auto num = std::get_if<number_t>(&value)) {
        ss << std::setprecision(15) << *num;
    } else if (auto num = std::get_if<integer_t>(&value)) {
        ss << *num;
    } else if (auto str = std::get_if<std::string>(&value)) {
        ss << util::escape(*str);
    } else {
        ss << "null";
    }
}

void stringifyArr(
    const List* list,
    std::stringstream& ss,
    int indent,
    const std::string& indentstr,
    bool nice
) {
    if (list == nullptr) {
        ss << "nullptr";
        return;
    }
    if (list->values.empty()) {
        ss << "[]";
        return;
    }
    ss << "[";
    for (size_t i = 0; i < list->size(); i++) {
        if (i > 0 || nice) {
            newline(ss, nice, indent, indentstr);
        }
        const Value& value = list->values[i];
        stringifyValue(value, ss, indent + 1, indentstr, nice);
        if (i + 1 < list->size()) {
            ss << ',';
        }
    }
    if (nice) {
        newline(ss, true, indent - 1, indentstr);
    }
    ss << ']';
}

void stringifyObj(
    const Map* obj,
    std::stringstream& ss,
    int indent,
    const std::string& indentstr,
    bool nice
) {
    if (obj == nullptr) {
        ss << "nullptr";
        return;
    }
    if (obj->values.empty()) {
        ss << "{}";
        return;
    }
    ss << "{";
    size_t index = 0;
    for (auto& entry : obj->values) {
        const std::string& key = entry.first;
        if (index > 0 || nice) {
            newline(ss, nice, indent, indentstr);
        }
        const Value& value = entry.second;
        ss << util::escape(key) << ": ";
        stringifyValue(value, ss, indent + 1, indentstr, nice);
        index++;
        if (index < obj->values.size()) {
            ss << ',';
        }
    }
    if (nice) {
        newline(ss, true, indent - 1, indentstr);
    }
    ss << '}';
}

std::string json::stringify(
    const Map* obj, bool nice, const std::string& indent
) {
    std::stringstream ss;
    stringifyObj(obj, ss, 1, indent, nice);
    return ss.str();
}

std::string json::stringify(
    const dynamic::List* arr, bool nice, const std::string& indent
) {
    std::stringstream ss;
    stringifyArr(arr, ss, 1, indent, nice);
    return ss.str();
}

std::string json::stringify(
    const dynamic::Value& value, bool nice, const std::string& indent
) {
    std::stringstream ss;
    stringifyValue(value, ss, 1, indent, nice);
    return ss.str();
}

Parser::Parser(std::string_view filename, std::string_view source)
    : BasicParser(filename, source) {
}

std::unique_ptr<Map> Parser::parse() {
    char next = peek();
    if (next != '{') {
        throw error("'{' expected");
    }
    return parseObject();
}

std::unique_ptr<Map> Parser::parseObject() {
    expect('{');
    auto obj = std::make_unique<Map>();
    auto& map = obj->values;
    while (peek() != '}') {
        if (peek() == '#') {
            skipLine();
            continue;
        }
        std::string key = parseName();
        char next = peek();
        if (next != ':') {
            throw error("':' expected");
        }
        pos++;
        map.insert(std::make_pair(key, parseValue()));
        next = peek();
        if (next == ',') {
            pos++;
        } else if (next == '}') {
            break;
        } else {
            throw error("',' expected");
        }
    }
    pos++;
    return obj;
}

std::unique_ptr<List> Parser::parseList() {
    expect('[');
    auto arr = std::make_unique<List>();
    auto& values = arr->values;
    while (peek() != ']') {
        if (peek() == '#') {
            skipLine();
            continue;
        }
        values.push_back(parseValue());

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
    return arr;
}

Value Parser::parseValue() {
    char next = peek();
    if (next == '-' || next == '+' || is_digit(next)) {
        return parseNumber();
    }
    if (is_identifier_start(next)) {
        std::string literal = parseName();
        if (literal == "true") {
            return true;
        } else if (literal == "false") {
            return false;
        } else if (literal == "inf") {
            return INFINITY;
        } else if (literal == "nan") {
            return NAN;
        }
        throw error("invalid literal ");
    }
    if (next == '{') {
        return Map_sptr(parseObject().release());
    }
    if (next == '[') {
        return List_sptr(parseList().release());
    }
    if (next == '"' || next == '\'') {
        pos++;
        return parseString(next);
    }
    throw error("unexpected character '" + std::string({next}) + "'");
}

dynamic::Map_sptr json::parse(
    std::string_view filename, std::string_view source
) {
    Parser parser(filename, source);
    return parser.parse();
}

dynamic::Map_sptr json::parse(std::string_view source) {
    return parse("<string>", source);
}
