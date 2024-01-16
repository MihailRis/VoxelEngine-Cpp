#include "json.h"

#include <math.h>
#include <sstream>
#include <iomanip>
#include <memory>

#include "commons.h"
#include "byte_utils.h"

using namespace json;

const int BJSON_END = 0x0;
const int BJSON_TYPE_DOCUMENT = 0x1;
const int BJSON_TYPE_LIST = 0x2;
const int BJSON_TYPE_BYTE = 0x3;
const int BJSON_TYPE_INT16 = 0x4;
const int BJSON_TYPE_INT32 = 0x5;
const int BJSON_TYPE_INT64 = 0x6;
const int BJSON_TYPE_NUMBER = 0x7;
const int BJSON_TYPE_STRING = 0x8;
const int BJSON_TYPE_BYTES = 0x9;
const int BJSON_TYPE_FALSE = 0xA;
const int BJSON_TYPE_TRUE = 0xB;
const int BJSON_TYPE_NULL = 0xC;
const int BJSON_TYPE_CDOCUMENT = 0x1F;

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

void stringifyObj(const JObject* obj, 
               std::stringstream& ss, 
               int indent, 
               const std::string& indentstr, 
               bool nice);

void stringify(const Value* value, 
               std::stringstream& ss, 
               int indent, 
               const std::string& indentstr, 
               bool nice) {
    if (value->type == valtype::object) {
        stringifyObj(value->value.obj, ss, indent, indentstr, nice);
    }
    else if (value->type == valtype::array) {
        std::vector<Value*>& list = value->value.arr->values;
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
        ss << std::setprecision(15);
        ss << value->value.decimal;
    } else if (value->type == valtype::integer) {
        ss << value->value.integer;
    } else if (value->type == valtype::string) {
        ss << escape_string(*value->value.str);
    }
}

void stringifyObj(const JObject* obj, 
                  std::stringstream& ss, 
                  int indent, 
                  const std::string& indentstr, 
                  bool nice) {
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

std::string json::stringify(
        const JObject* obj, 
        bool nice, 
        const std::string& indent) {
    std::stringstream ss;
    stringifyObj(obj, ss, 1, indent, nice);
    return ss.str();
}

static void to_binary(ByteBuilder& builder, const Value* value) {
    switch (value->type) {
        case valtype::object: {
            std::vector<ubyte> bytes = to_binary(value->value.obj);
            builder.put(bytes.data(), bytes.size());
            break;
        }
        case valtype::array:
            builder.put(BJSON_TYPE_LIST);
            for (Value* element : value->value.arr->values) {
                to_binary(builder, element);
            }
            builder.put(BJSON_END);
            break;
        case valtype::integer: {
            int64_t val = value->value.integer;
            if (val >= 0 && val <= 255) {
                builder.put(BJSON_TYPE_BYTE);
                builder.put(val);
            } else if (val >= INT16_MIN && val <= INT16_MAX){
                builder.put(BJSON_TYPE_INT16);
                builder.putInt16(val);
            } else if (val >= INT32_MIN && val <= INT32_MAX) {
                builder.put(BJSON_TYPE_INT32);
                builder.putInt32(val);
            } else {
                builder.put(BJSON_TYPE_INT64);
                builder.putInt64(val);
            }
            break;
        }
        case valtype::number:
            builder.put(BJSON_TYPE_NUMBER);
            builder.putFloat64(value->value.decimal);
            break;
        case valtype::boolean:
            builder.put(BJSON_TYPE_FALSE + value->value.boolean);
            break;
        case valtype::string:
            builder.put(BJSON_TYPE_STRING);
            builder.put(*value->value.str);
            break;
    }
}

static JArray* array_from_binary(ByteReader& reader);
static JObject* object_from_binary(ByteReader& reader);

std::vector<ubyte> json::to_binary(const JObject* obj) {
    ByteBuilder builder;
    // type byte
    builder.put(BJSON_TYPE_DOCUMENT);
    // document size
    builder.putInt32(0);

    // writing entries
    for (auto& entry : obj->map) {
        builder.putCStr(entry.first.c_str());
        to_binary(builder, entry.second);
    }
    // terminating byte
    builder.put(BJSON_END);

    // updating document size
    builder.setInt32(1, builder.size());
    return builder.build();
}

static Value* value_from_binary(ByteReader& reader) {
    ubyte typecode = reader.get();
    valtype type;
    valvalue val;
    switch (typecode) {
        case BJSON_TYPE_DOCUMENT:
            type = valtype::object;
            reader.getInt32();
            val.obj = object_from_binary(reader);
            break;
        case BJSON_TYPE_LIST:
            type = valtype::array;
            val.arr = array_from_binary(reader);
            break;
        case BJSON_TYPE_BYTE:
            type = valtype::integer;
            val.integer = reader.get();
            break;
        case BJSON_TYPE_INT16:
            type = valtype::integer;
            val.integer = reader.getInt16();
            break;
        case BJSON_TYPE_INT32:
            type = valtype::integer;
            val.integer = reader.getInt32();
            break;
        case BJSON_TYPE_INT64:
            type = valtype::integer;
            val.integer = reader.getInt64();
            break;
        case BJSON_TYPE_NUMBER:
            type = valtype::number;
            val.decimal = reader.getFloat64();
            break;
        case BJSON_TYPE_FALSE:
        case BJSON_TYPE_TRUE:
            type = valtype::boolean;
            val.boolean = typecode - BJSON_TYPE_FALSE;
            break;
        case BJSON_TYPE_STRING:
            type = valtype::string;
            val.str = new std::string(reader.getString());
            break;
        default:
            throw std::runtime_error(
                  "type "+std::to_string(typecode)+" is not supported");
    }
    return new Value(type, val);
}

static JArray* array_from_binary(ByteReader& reader) {
    auto array = std::make_unique<JArray>();
    auto& items = array->values;
    while (reader.peek() != BJSON_END) {
        items.push_back(value_from_binary(reader));
    }
    reader.get();
    return array.release();
}

static JObject* object_from_binary(ByteReader& reader) {
    auto obj = std::make_unique<JObject>();
    auto& map = obj->map;
    while (reader.peek() != BJSON_END) {
        const char* key = reader.getCString();
        Value* value = value_from_binary(reader);
        map.insert(std::make_pair(key, value));
    }
    reader.get();
    return obj.release();
}

JObject* json::from_binary(const ubyte* src, size_t size) {
    ByteReader reader(src, size);
    std::unique_ptr<Value> value (value_from_binary(reader));
    if (value->type != valtype::object) {
        throw std::runtime_error("root value is not an object");
    }
    JObject* obj = value->value.obj;
    value->value.obj = nullptr;
    return obj;
}

JArray::~JArray() {
    for (auto value : values) {
        delete value;
    }
}

std::string JArray::str(size_t index) const {
    const auto& val = values[index];
    switch (val->type) {
        case valtype::string: return *val->value.str;
        case valtype::boolean: return val->value.boolean ? "true" : "false";
        case valtype::number: return std::to_string(val->value.decimal);
        case valtype::integer: return std::to_string(val->value.integer);
        default:
            throw std::runtime_error("type error");
    }
}

double JArray::num(size_t index) const {
    const auto& val = values[index];
    switch (val->type) {
        case valtype::number: return val->value.decimal;
        case valtype::integer: return val->value.integer;
        case valtype::string: return std::stoll(*val->value.str);
        case valtype::boolean: return val->value.boolean;
        default:
            throw std::runtime_error("type error");
    }
}

int64_t JArray::integer(size_t index) const {
    const auto& val = values[index];
    switch (val->type) {
        case valtype::number: return val->value.decimal;
        case valtype::integer: return val->value.integer;
        case valtype::string: return std::stoll(*val->value.str);
        case valtype::boolean: return val->value.boolean;
        default:
            throw std::runtime_error("type error");
    }
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

JArray& JArray::put(std::string value) {
    valvalue val;
    val.str = new std::string(value);
    values.push_back(new Value(valtype::string, val));
    return *this;
}

JArray& JArray::put(uint value) {
    return put((int64_t)value);
}

JArray& JArray::put(int value) {
    return put((int64_t)value);
}

JArray& JArray::put(int64_t value) {
    valvalue val;
    val.integer = value;
    values.push_back(new Value(valtype::integer, val));
    return *this;
}

JArray& JArray::put(uint64_t value) {
    return put((int64_t)value);
}

JArray& JArray::put(double value) {
    valvalue val;
    val.decimal = value;
    values.push_back(new Value(valtype::number, val));
    return *this;
}

JArray& JArray::put(float value) {
    return put((double)value);
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

JArray& JArray::putArray() {
    JArray* arr = new JArray();
    put(arr);
    return *arr;
}

JObject& JArray::putObj() {
    JObject* obj = new JObject();
    put(obj);
    return *obj;
}

void JArray::remove(size_t index) {
    values.erase(values.begin() + index);
}

JObject::~JObject() {
    for (auto entry : map) {
        delete entry.second;
    }
}

void JObject::str(std::string key, std::string& dst) const {
    dst = getStr(key, dst);
}

std::string JObject::getStr(std::string key, const std::string& def) const {
    auto found = map.find(key);
    if (found == map.end())
        return def;
    auto& val = found->second;
    switch (val->type) {
        case valtype::string: return *val->value.str;
        case valtype::boolean: return val->value.boolean ? "true" : "false";
        case valtype::number: return std::to_string(val->value.decimal);
        case valtype::integer: return std::to_string(val->value.integer);
        default: throw std::runtime_error("type error");
    } 
}

double JObject::getNum(std::string key, double def) const {
    auto found = map.find(key);
    if (found == map.end())
        return def;
    auto& val = found->second;
    switch (val->type) {
        case valtype::number: return val->value.decimal;
        case valtype::integer: return val->value.integer;
        case valtype::string: return std::stoull(*val->value.str);
        case valtype::boolean: return val->value.boolean;
        default: throw std::runtime_error("type error");
    }
}

int64_t JObject::getInteger(std::string key, int64_t def) const {
    auto found = map.find(key);
    if (found == map.end())
        return def;
    auto& val = found->second;
    switch (val->type) {
        case valtype::number: return val->value.decimal;
        case valtype::integer: return val->value.integer;
        case valtype::string: return std::stoull(*val->value.str);
        case valtype::boolean: return val->value.boolean;
        default: throw std::runtime_error("type error");
    }
}

void JObject::num(std::string key, double& dst) const {
    dst = getNum(key, dst);
}

void JObject::num(std::string key, float& dst) const {
    dst = getNum(key, dst);
}

void JObject::num(std::string key, ubyte& dst) const {
    dst = getInteger(key, dst);
}

void JObject::num(std::string key, int& dst) const {
    dst = getInteger(key, dst);
}

void JObject::num(std::string key, int64_t& dst) const {
    dst = getInteger(key, dst);
}

void JObject::num(std::string key, uint64_t& dst) const {
    dst = getInteger(key, dst);
}

void JObject::num(std::string key, uint& dst) const {
    dst = getInteger(key, dst);
}

JObject* JObject::obj(std::string key) const {
    auto found = map.find(key);
    if (found != map.end()) {
        auto& val = found->second;
        if (val->type != valtype::object)
            return nullptr;
        return val->value.obj;
    }
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

JObject& JObject::put(std::string key, uint value) {
    return put(key, (int64_t)value);
}

JObject& JObject::put(std::string key, int value) {
    return put(key, (int64_t)value);
}

JObject& JObject::put(std::string key, int64_t value) {
    auto found = map.find(key);
    if (found != map.end())  found->second;
    valvalue val;
    val.integer = value;
    map.insert(std::make_pair(key, new Value(valtype::integer, val)));
    return *this;
}

JObject& JObject::put(std::string key, uint64_t value) {
    return put(key, (int64_t)value);
}

JObject& JObject::put(std::string key, float value) {
    return put(key, (double)value);
}

JObject& JObject::put(std::string key, double value) {
    auto found = map.find(key);
    if (found != map.end()) delete found->second;
    valvalue val;
    val.decimal = value;
    map.insert(std::make_pair(key, new Value(valtype::number, val)));
    return *this;
}

JObject& JObject::put(std::string key, std::string value){
    auto found = map.find(key);
    if (found != map.end()) delete found->second;
    valvalue val;
    val.str = new std::string(value);
    map.insert(std::make_pair(key, new Value(valtype::string, val)));
    return *this;
}

JObject& JObject::put(std::string key, const char* value) {
    return put(key, std::string(value));
}

JObject& JObject::put(std::string key, JObject* value){
    auto found = map.find(key);
    if (found != map.end()) delete found->second;
    valvalue val;
    val.obj = value;
    map.insert(std::make_pair(key, new Value(valtype::object, val)));
    return *this;
}

JObject& JObject::put(std::string key, JArray* value){
    auto found = map.find(key);
    if (found != map.end()) delete found->second;
    valvalue val;
    val.arr = value;
    map.insert(std::make_pair(key, new Value(valtype::array, val)));
    return *this;
}

JObject& JObject::put(std::string key, bool value){
    auto found = map.find(key);
    if (found != map.end()) delete found->second;
    valvalue val;
    val.boolean = value;
    map.insert(std::make_pair(key, new Value(valtype::boolean, val)));
    return *this;
}

JArray& JObject::putArray(std::string key) {
    JArray* arr = new JArray();
    put(key, arr);
    return *arr;
}

JObject& JObject::putObj(std::string key) {
    JObject* obj = new JObject();
    put(key, obj);
    return *obj;
}

bool JObject::has(std::string key) {
    return map.find(key) != map.end();
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

Parser::Parser(std::string filename, std::string source) 
      : BasicParser(filename, source) {    
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
    auto obj = std::make_unique<JObject>();
    auto& map = obj->map;
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

JArray* Parser::parseArray() {
    expect('[');
    auto arr = std::make_unique<JArray>();
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
    return arr.release();
}

Value* Parser::parseValue() {
    char next = peek();
    valvalue val;
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
        val.obj = parseObject();
        return new Value(valtype::object, val);
    }
    if (next == '[') {
        val.arr = parseArray();
        return new Value(valtype::array, val);
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

JObject* json::parse(std::string filename, std::string source) {
    Parser parser(filename, source);
    return parser.parse();
}

JObject* json::parse(std::string source) {
    return parse("<string>", source);
}
