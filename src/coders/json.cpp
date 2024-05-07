#include "json.hpp"

#include "../data/dynamic.hpp"
#include "../util/stringutil.hpp"

#include <math.h>
#include <sstream>
#include <iomanip>
#include <memory>

using namespace json;
using namespace dynamic;

inline void newline(
    std::stringstream& ss, 
    bool nice, uint indent, 
    const std::string& indentstr
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

void stringify(
    const Value* value, 
    std::stringstream& ss, 
    int indent, 
    const std::string& indentstr, 
    bool nice
);

void stringifyObj(
    const Map* obj, 
    std::stringstream& ss, 
    int indent, 
    const std::string& indentstr, 
    bool nice
);

void stringify(
    const Value* value, 
    std::stringstream& ss, 
    int indent, 
    const std::string& indentstr, 
    bool nice
) {
    if (auto map = std::get_if<Map*>(&value->value)) {
        stringifyObj(*map, ss, indent, indentstr, nice);
    }
    else if (auto listptr = std::get_if<List*>(&value->value)) {
        auto list = *listptr;
        if (list->size() == 0) {
            ss << "[]";
            return;
        }
        ss << '[';
        for (uint i = 0; i < list->size(); i++) {
            Value* value = list->get(i);
            if (i > 0 || nice) {
                newline(ss, nice, indent, indentstr);
            }
            stringify(value, ss, indent+1, indentstr, nice);
            if (i + 1 < list->size()) {
                ss << ',';
            }
        }
        if (nice) {
            newline(ss, true, indent - 1, indentstr);
        }
        ss << ']';
    } else if (auto flag = std::get_if<bool>(&value->value)) {
        ss << (*flag ? "true" : "false");
    } else if (auto num = std::get_if<number_t>(&value->value)) {
        ss << std::setprecision(15) << *num;
    } else if (auto num = std::get_if<integer_t>(&value->value)) {
        ss << *num;
    } else if (auto str = std::get_if<std::string>(&value->value)) {
        ss << util::escape(*str);
    }
}

void stringifyObj(
    const Map* obj, 
    std::stringstream& ss, 
    int indent, 
    const std::string& indentstr, 
    bool nice
) {
    if (obj->values.empty()) {
        ss << "{}";
        return;
    }
    ss << "{";
    uint index = 0;
    for (auto& entry : obj->values) {
        const std::string& key = entry.first;
        if (index > 0 || nice) {
            newline(ss, nice, indent, indentstr);
        }
        Value* value = entry.second.get();
        ss << util::escape(key) << ": ";
        stringify(value, ss, indent+1, indentstr, nice);
        index++;
        if (index < obj->values.size()) {
            ss << ',';
        }
    }
    if (nice) {
        newline(ss, true, indent-1, indentstr);
    }
    ss << '}';
}

std::string json::stringify(
    const Map* obj, 
    bool nice, 
    const std::string& indent
) {
    std::stringstream ss;
    stringifyObj(obj, ss, 1, indent, nice);
    return ss.str();
}

Parser::Parser(const std::string& filename, const std::string& source) 
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

std::unique_ptr<Value> Parser::parseValue() {
    char next = peek();
    dynamic::valvalue val;
    if (next == '-' || next == '+') {
        pos++;
        number_u num;
        if (parseNumber(next == '-' ? -1 : 1, num)) {
            val = std::get<integer_t>(num);
        } else {
            val = std::get<number_t>(num);
        }
        return std::make_unique<Value>(val);
    }
    if (is_identifier_start(next)) {
        std::string literal = parseName();
        if (literal == "true") {
            return Value::boolean(true);
        } else if (literal == "false") {
            return Value::boolean(false);
        } else if (literal == "inf") {
            return Value::of(INFINITY);
        } else if (literal == "nan") {
            return Value::of(NAN);
        }
        throw error("invalid literal ");
    }
    if (next == '{') {
        val = parseObject().release();
        return std::make_unique<Value>(val);
    }
    if (next == '[') {
        val = parseList().release();
        return std::make_unique<Value>(val);
    }
    if (is_digit(next)) {
        number_u num;
        if (parseNumber(1, num)) {
            val = std::get<integer_t>(num);
        } else {
            val = std::get<number_t>(num);
        }
        return std::make_unique<Value>(val);  
    }
    if (next == '"' || next == '\'') {
        pos++;
        val = parseString(next);
        return std::make_unique<Value>(val);
    }
    throw error("unexpected character '"+std::string({next})+"'");
}

std::unique_ptr<Map> json::parse(const std::string& filename, const std::string& source) {
    Parser parser(filename, source);
    return parser.parse();
}

std::unique_ptr<Map> json::parse(const std::string& source) {
    return parse("<string>", source);
}
