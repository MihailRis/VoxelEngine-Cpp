#include "json.h"

#include <math.h>
#include <sstream>
#include <memory>

#include "commons.h"

using namespace json;

using std::string;
using std::vector;
using std::unique_ptr;
using std::unordered_map;
using std::stringstream;
using std::make_pair;

inline void newline(std::stringstream& ss, bool nice, uint indent, const std::string indentstr) {
    if (nice) {
        ss << "\n";
        for (uint i = 0; i < indent; i++) {
            ss << indentstr;
        }
    } else {
        ss << ' ';
    }
}

void stringify(Value* value, stringstream& ss, int indent, string indentstr, bool nice);
void stringifyObj(JObject* obj, stringstream& ss, int indent, string indentstr, bool nice);

void stringify(Value* value, stringstream& ss, int indent, string indentstr, bool nice) {
    if (value->type == valtype::object) {
        stringifyObj(value->value.obj, ss, indent, indentstr, nice);
    }
    else if (value->type == valtype::array) {
        vector<Value*>& list = value->value.arr->values;
        if (list.empty()) {
            ss << "[]";
            return;
        }
        ss << '[';
        for (uint i = 0; i < list.size(); i++) {
            Value* value = list[i];
            if (i > 0 || nice) {
                newline(ss, nice, indent, indentstr);
            }
            stringify(value, ss, indent+1, indentstr, nice);
            if (i + 1 < list.size()) {
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
        ss << value->value.num;
    } else if (value->type == valtype::string) {
        ss << escape_string(*value->value.str);
    }
}

void stringifyObj(JObject* obj, stringstream& ss, int indent, string indentstr, bool nice) {
    if (obj->map.empty()) {
        ss << "{}";
        return;
    }
    ss << "{";
    uint index = 0;
    for (auto entry : obj->map) {
        const std::string& key = entry.first;
        if (index > 0 || nice) {
            newline(ss, nice, indent, indentstr);
        }
        Value* value = entry.second;
        ss << escape_string(key) << ": ";
        stringify(value, ss, indent+1, indentstr, nice);
        index++;
        if (index < obj->map.size()) {
            ss << ',';
        }
    }
    if (nice) {
        newline(ss, true, indent-1, indentstr);
    }
    ss << '}';
}

string json::stringify(JObject* obj, bool nice, string indent) {
    stringstream ss;
    stringifyObj(obj, ss, 1, indent, nice);
    return ss.str();
}


JArray::~JArray() {
    for (auto value : values) {
        delete value;
    }
}

std::string JArray::str(size_t index) const {
    return *values[index]->value.str;
}

double JArray::num(size_t index) const {
    return values[index]->value.num;
}

JObject* JArray::obj(size_t index) const {
    return values[index]->value.obj;
}

JArray* JArray::arr(size_t index) const {
    return values[index]->value.arr;
}

bool JArray::flag(size_t index) const {
    return values[index]->value.boolean;
}

JArray& JArray::put(string value) {
    valvalue val;
    val.str = new string(value);
    values.push_back(new Value(valtype::string, val));
    return *this;
}

JArray& JArray::put(uint value) {
    return put((double)value);
}

JArray& JArray::put(int value) {
    return put((double)value);
}

JArray& JArray::put(double value) {
    valvalue val;
    val.num = value;
    values.push_back(new Value(valtype::number, val));
    return *this;
}

JArray& JArray::put(float value) {
    valvalue val;
    val.num = value;
    values.push_back(new Value(valtype::number, val));
    return *this;
}

JArray& JArray::put(bool value) {
    valvalue val;
    val.boolean = value;
    values.push_back(new Value(valtype::boolean, val));
    return *this;
}

JArray& JArray::put(JObject* value) {
    valvalue val;
    val.obj = value;
    values.push_back(new Value(valtype::object, val));
    return *this;
}

JArray& JArray::put(JArray* value) {
    valvalue val;
    val.arr = value;
    values.push_back(new Value(valtype::array, val));
    return *this;
}

JObject::~JObject() {
    for (auto entry : map) {
        delete entry.second;
    }
}

void JObject::str(std::string key, std::string& dst) const {
    auto found = map.find(key);
    if (found != map.end())
        dst = *found->second->value.str;
}

void JObject::num(std::string key, double& dst) const {
    auto found = map.find(key);
    if (found != map.end())
        dst = found->second->value.num;
}

void JObject::num(std::string key, float& dst) const {
    auto found = map.find(key);
    if (found != map.end())
        dst = found->second->value.num;
}

void JObject::num(std::string key, int& dst) const {
    auto found = map.find(key);
    if (found != map.end())
        dst = found->second->value.num;
}

void JObject::num(std::string key, uint& dst) const {
    auto found = map.find(key);
    if (found != map.end())
        dst = found->second->value.num;
}

JObject* JObject::obj(std::string key) const {
    auto found = map.find(key);
    if (found != map.end())
        return found->second->value.obj;
    return nullptr;
}

JArray* JObject::arr(std::string key) const {
    auto found = map.find(key);
    if (found != map.end())
        return found->second->value.arr;
    return nullptr;
}

void JObject::flag(std::string key, bool& dst) const {
    auto found = map.find(key);
    if (found != map.end())
        dst = found->second->value.boolean;
}

JObject& JObject::put(string key, uint value) {
    return put(key, (double)value);
}

JObject& JObject::put(string key, int value) {
    return put(key, (double)value);
}

JObject& JObject::put(string key, float value) {
    return put(key, (double)value);
}

JObject& JObject::put(string key, double value) {
    auto found = map.find(key);
    if (found != map.end()) delete found->second;
    valvalue val;
    val.num = value;
    map.insert(make_pair(key, new Value(valtype::number, val)));
    return *this;
}

JObject& JObject::put(string key, string value){
    auto found = map.find(key);
    if (found != map.end()) delete found->second;
    valvalue val;
    val.str = new string(value);
    map.insert(make_pair(key, new Value(valtype::string, val)));
    return *this;
}

JObject& JObject::put(string key, JObject* value){
    auto found = map.find(key);
    if (found != map.end()) delete found->second;
    valvalue val;
    val.obj = value;
    map.insert(make_pair(key, new Value(valtype::object, val)));
    return *this;
}

JObject& JObject::put(string key, JArray* value){
    auto found = map.find(key);
    if (found != map.end()) delete found->second;
    valvalue val;
    val.arr = value;
    map.insert(make_pair(key, new Value(valtype::array, val)));
    return *this;
}

JObject& JObject::put(string key, bool value){
    auto found = map.find(key);
    if (found != map.end()) delete found->second;
    valvalue val;
    val.boolean = value;
    map.insert(make_pair(key, new Value(valtype::boolean, val)));
    return *this;
}

Value::Value(valtype type, valvalue value) : type(type), value(value) {
}

Value::~Value() {
    switch (type) {
        case valtype::object: delete value.obj; break;
        case valtype::array: delete value.arr; break;
        case valtype::string: delete value.str; break;
        default:
            break;
    }
}

Parser::Parser(string filename, string source) : BasicParser(filename, source) {    
}

JObject* Parser::parse() {
    char next = peek();
    if (next != '{') {
        throw error("'{' expected");
    }
    return parseObject();
}

JObject* Parser::parseObject() {
    expect('{');
    unique_ptr<JObject> obj(new JObject());
    unordered_map<string, Value*>& map = obj->map;
    while (peek() != '}') {
        string key = parseName();
        char next = peek();
        if (next != ':') {
            throw error("':' expected");
        }
        pos++;
        map.insert(make_pair(key, parseValue()));
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

JArray* Parser::parseArray() {
    expect('[');
    unique_ptr<JArray> arr(new JArray());
    vector<Value*>& values = arr->values;
    while (peek() != ']') {
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
    return arr.release();
}

Value* Parser::parseValue() {
    char next = peek();
    valvalue val;
    if (is_identifier_start(next)) {
        string literal = parseName();
        if (literal == "true") {
            val.boolean = true;
            return new Value(valtype::boolean, val);
        } else if (literal == "false") {
            val.boolean = false;
            return new Value(valtype::boolean, val);
        } else if (literal == "inf") {
            val.num = INFINITY;
            return new Value(valtype::number, val);
        } else if (literal == "nan") {
            val.num = NAN;
            return new Value(valtype::number, val);
        }
        throw error("invalid literal");
    }
    if (next == '{') {
        val.obj = parseObject();
        return new Value(valtype::object, val);
    }
    if (next == '[') {
        val.arr = parseArray();
        return new Value(valtype::array, val);
    }
    if (next == '-' || next == '+') {
        pos++;
        val.num = parseNumber(next == '-' ? -1 : 1);
        return new Value(valtype::number, val);
    }
    if (is_digit(next)) {
        val.num = parseNumber(1);
        return new Value(valtype::number, val);  
    }
    if (next == '"' || next == '\'') {
        pos++;
        val.str = new string(parseString(next));
        return new Value(valtype::string, val);
    }
    throw error("unexpected character '"+string({next})+"'");
}

JObject* json::parse(std::string filename, std::string source) {
    Parser parser(filename, source);
    return parser.parse();
}

JObject* json::parse(std::string source) {
    return parse("<string>", source);
}