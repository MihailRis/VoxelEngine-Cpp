#include "json.h"

#include <math.h>
#include <sstream>
#include <memory>

using namespace json;

using std::string;
using std::vector;
using std::unique_ptr;
using std::unordered_map;
using std::stringstream;
using std::make_pair;

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

inline int char2int(int c) {
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

inline number_t power(number_t base, int64_t power) {
    number_t result = 1.0;
    for (int64_t i = 0; i < power; i++) {
        result *= base;
    }
    return result;
}

string json::escape(string s) {
    std::stringstream ss;
    ss << '"';
    for (char c : s) {
        switch (c) {
            case '\n': ss << "\\n"; break;
            case '\r': ss << "\\r"; break;
            case '\t': ss << "\\t"; break;
            case '\f': ss << "\\f"; break;
            case '\b': ss << "\\b"; break;
            case '"': ss << "\\\""; break;
            case '\\': ss << "\\\\"; break;
            default:
                if (c < ' ') {
                    ss << "\\" << std::oct << (int)c;
                    break;
                }
                ss << c;
                break;
        }
    }
    ss << '"';
    return ss.str();
}

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
        ss << escape(*value->value.str);
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
        ss << escape(key) << ": ";
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


parsing_error::parsing_error(string message, 
                string filename, 
                string source, 
                uint pos, 
                uint line, 
                uint linestart)
    : std::runtime_error(message), filename(filename), source(source), 
      pos(pos), line(line), linestart(linestart) {
}

string parsing_error::errorLog() const {
    std::stringstream ss;
    uint linepos = pos - linestart;
    ss << "parsing error in file '" << filename;
    ss << "' at " << (line+1) << ":" << linepos << ": " << this->what() << "\n";
    size_t end = source.find("\n", linestart);
    if (end == string::npos) {
        end = source.length();
    }
    ss << source.substr(linestart, end-linestart) << "\n";
    for (uint i = 0; i < linepos; i++) {
        ss << " ";
    }
    ss << "^";
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

number_t JArray::num(size_t index) const {
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
    return put((number_t)value);
}

JArray& JArray::put(int value) {
    return put((number_t)value);
}

JArray& JArray::put(number_t value) {
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

void JObject::num(std::string key, number_t& dst) const {
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
    return put(key, (number_t)value);
}

JObject& JObject::put(string key, int value) {
    return put(key, (number_t)value);
}

JObject& JObject::put(string key, float value) {
    return put(key, (number_t)value);
}

JObject& JObject::put(string key, number_t value) {
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

Parser::Parser(string filename, string source) : filename(filename), source(source) {    
}

JObject* Parser::parse() {
    char next = peek();
    if (next != '{') {
        throw error("'{' expected");
    }
    return parseObject();
}

bool Parser::hasNext() {
    return pos < source.length();
}

char Parser::nextChar() {
    if (!hasNext()) {
        throw error("unexpected end");
    }
    return source[pos++];
}

void Parser::expect(char expected) {
    char c = peek();
    if (c != expected) {
        throw error("'"+string({expected})+"' expected");
    }
    pos++;
}

char Parser::peek() {
    skipWhitespace();
    if (pos >= source.length()) {
        throw error("unexpected end");
    }
    return source[pos];
}

parsing_error Parser::error(std::string message) {
    return parsing_error(message, filename, source, pos, line, linestart);
}

void Parser::skipWhitespace() {
    while (hasNext()) {
        char next = source[pos];
        if (next == '\n') {
            line++;
            linestart = ++pos;
            continue;
        }
        if (is_whitespace(next)) {
            pos++;
        } else {
            break;
        }
    }
}

string Parser::parseName() {
    char c = peek();
    if (!is_identifier_start(c)) {
        if (c == '"') {
            pos++;
            return parseString(c);
        }
        throw error("identifier expected");
    }
    int start = pos;
    while (hasNext() && is_identifier_part(source[pos])) {
        pos++;
    }
    return source.substr(start, pos-start);
}

int64_t Parser::parseSimpleInt(int base) {
    char c = peek();
    int index = char2int(c);
    if (index == -1 || index >= base) {
        throw error("invalid number literal");
    }
    int64_t value = index;
    pos++;
    while (hasNext()) {
        c = source[pos];
        while (c == '_') {
            c = source[++pos];
        }
        index = char2int(c);
        if (index == -1 || index >= base) {
            return value;
        }
        value *= base;
        value += index;
        pos++;
    }
    return value;
}

number_t Parser::parseNumber(int sign) {
    char c = peek();
    int base = 10;
    if (c == '0' && pos + 1 < source.length() && 
          (base = is_box(source[pos+1])) != 10) {
        pos += 2;
        return parseSimpleInt(base);
    }
    int64_t value = parseSimpleInt(base);
    if (!hasNext()) {
        return value * sign;
    }
    c = source[pos];
    if (c == 'e' || c == 'E') {
        pos++;
        int s = 1;
        if (peek() == '-') {
            s = -1;
            pos++;
        } else if (peek() == '+'){
            pos++;
        }
        return sign * value * power(10.0, s * parseSimpleInt(10));
    }
    if (c == '.') {
        pos++;
        int64_t expo = 1;
        while (hasNext() && source[pos] == '0') {
            expo *= 10;
            pos++;
        }
        int64_t afterdot = 0;
        if (hasNext() && is_digit(source[pos])) {
            afterdot = parseSimpleInt(10);
        }
        expo *= power(10, fmax(0, log10(afterdot) + 1));
        c = source[pos];

        number_t dvalue = (value + (afterdot / (number_t)expo));
        if (c == 'e' || c == 'E') {
            pos++;
            int s = 1;
            if (peek() == '-') {
                s = -1;
                pos++;
            } else if (peek() == '+'){
                pos++;
            }
            return sign * dvalue * power(10.0, s * parseSimpleInt(10));
        }
        return dvalue;
    }
    return value;
}

string Parser::parseString(char quote) {
    std::stringstream ss;
    while (hasNext()) {
        char c = source[pos];
        if (c == quote) {
            pos++;
            return ss.str();
        }
        if (c == '\\') {
            pos++;
            c = nextChar();
            if (c >= '0' && c <= '7') {
                pos--;
                ss << (char)parseSimpleInt(8);
                continue;
            }
            switch (c) {
                case 'n': ss << '\n'; break;
                case 'r': ss << '\r'; break;
                case 'b': ss << '\b'; break;
                case 't': ss << '\t'; break;
                case 'f': ss << '\f'; break;
                case '\'': ss << '\\'; break;
                case '"': ss << '"'; break;
                case '\\': ss << '\\'; break;
                case '/': ss << '/'; break;
                case '\n': pos++; continue;
                default:
                    throw error("'\\" + string({c}) + "' is an illegal escape");
            }
            continue;
        }
        if (c == '\n') {
            throw error("non-closed string literal");
        }
        ss << c;
        pos++;
    }
    throw error("unexpected end");
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
        }
        if (literal == "false") {
            val.boolean = false;
            return new Value(valtype::boolean, val);
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