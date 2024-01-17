#include "json.h"

#include <math.h>
#include <sstream>
#include <iomanip>
#include <memory>

#include "commons.h"
#include "../data/dynamic.h"

using namespace json;
using namespace dynamic;

inline void newline(std::stringstream& ss, 
                    bool nice, uint indent, 
                    const std::string& indentstr) {
    if (nice) {
        ss << "\n";
        for (uint i = 0; i < indent; i++) {
            ss << indentstr;
        }
    } else {
        ss << ' ';
    }
}

void stringify(const Value* value, 
               std::stringstream& ss, 
               int indent, 
               const std::string& indentstr, 
               bool nice);

void stringifyObj(const Map* obj, 
               std::stringstream& ss, 
               int indent, 
               const std::string& indentstr, 
               bool nice);

void stringify(const Value* value, 
               std::stringstream& ss, 
               int indent, 
               const std::string& indentstr, 
               bool nice) {
    if (value->type == valtype::map) {
        stringifyObj(value->value.map, ss, indent, indentstr, nice);
    }
    else if (value->type == valtype::list) {
        auto list = value->value.list;
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
    } else if (value->type == valtype::boolean) {
        ss << (value->value.boolean ? "true" : "false");
    } else if (value->type == valtype::number) {
        ss << std::setprecision(15);
        ss << value->value.decimal;
    } else if (value->type == valtype::integer) {
        ss << value->value.integer;
    } else if (value->type == valtype::string) {
        ss << escape_string(*value->value.str);
    }
}

void stringifyObj(const Map* obj, 
                  std::stringstream& ss, 
                  int indent, 
                  const std::string& indentstr, 
                  bool nice) {
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
        ss << escape_string(key) << ": ";
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
        const std::string& indent) {
    std::stringstream ss;
    stringifyObj(obj, ss, 1, indent, nice);
    return ss.str();
}

Parser::Parser(std::string filename, std::string source) 
      : BasicParser(filename, source) {    
}

Map* Parser::parse() {
    char next = peek();
    if (next != '{') {
        throw error("'{' expected");
    }
    return parseObject();
}

Map* Parser::parseObject() {
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
    return obj.release();
}

List* Parser::parseList() {
    expect('[');
    auto arr = std::make_unique<List>();
    auto& values = arr->values;
    while (peek() != ']') {
        if (peek() == '#') {
            skipLine();
            continue;
        }
        values.push_back(std::unique_ptr<Value>(parseValue()));

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
    return arr.release();
}

Value* Parser::parseValue() {
    char next = peek();
    dynamic::valvalue val;
    if (next == '-' || next == '+') {
        pos++;
        number_u num;
        valtype type;
        if (parseNumber(next == '-' ? -1 : 1, num)) {
            val.integer = num.ival;
            type = valtype::integer;
        } else {
            val.decimal = num.fval;
            type = valtype::number;
        }
        return new Value(type, val);
    }
    if (is_identifier_start(next)) {
        std::string literal = parseName();
        if (literal == "true") {
            val.boolean = true;
            return new Value(valtype::boolean, val);
        } else if (literal == "false") {
            val.boolean = false;
            return new Value(valtype::boolean, val);
        } else if (literal == "inf") {
            val.decimal = INFINITY;
            return new Value(valtype::number, val);
        } else if (literal == "nan") {
            val.decimal = NAN;
            return new Value(valtype::number, val);
        }
        throw error("invalid literal ");
    }
    if (next == '{') {
        val.map = parseObject();
        return new Value(valtype::map, val);
    }
    if (next == '[') {
        val.list = parseList();
        return new Value(valtype::list, val);
    }
    if (is_digit(next)) {
        number_u num;
        valtype type;
        if (parseNumber(1, num)) {
            val.integer = num.ival;
            type = valtype::integer;
        } else {
            val.decimal = num.fval;
            type = valtype::number;
        }
        return new Value(type, val);  
    }
    if (next == '"' || next == '\'') {
        pos++;
        val.str = new std::string(parseString(next));
        return new Value(valtype::string, val);
    }
    throw error("unexpected character '"+std::string({next})+"'");
}

std::unique_ptr<Map> json::parse(std::string filename, std::string source) {
    Parser parser(filename, source);
    return std::unique_ptr<Map>(parser.parse());
}

std::unique_ptr<Map> json::parse(std::string source) {
    return parse("<string>", source);
}
