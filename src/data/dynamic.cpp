#include "dynamic.h"

#include <stdexcept>

using namespace dynamic;

List::~List() {
}

std::string List::str(size_t index) const {
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

double List::num(size_t index) const {
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

int64_t List::integer(size_t index) const {
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

Map* List::map(size_t index) const {
    if (values[index]->type != valtype::map) {
        throw std::runtime_error("type error");
    }
    return values[index]->value.map;
}

List* List::list(size_t index) const {
    if (values[index]->type != valtype::list) {
        throw std::runtime_error("type error");
    }
    return values[index]->value.list;
}

bool List::flag(size_t index) const {
    const auto& val = values[index];
    switch (val->type) {
        case valtype::integer: return val->value.integer;
        case valtype::boolean: return val->value.boolean;
        default:
            throw std::runtime_error("type error");
    }
}

List& List::put(std::string value) {
    valvalue val;
    val.str = new std::string(value);
    values.push_back(std::make_unique<Value>(valtype::string, val));
    return *this;
}

List& List::put(uint value) {
    return put((int64_t)value);
}

List& List::put(int value) {
    return put((int64_t)value);
}

List& List::put(int64_t value) {
    valvalue val;
    val.integer = value;
    values.push_back(std::make_unique<Value>(valtype::integer, val));
    return *this;
}

List& List::put(uint64_t value) {
    return put((int64_t)value);
}

List& List::put(double value) {
    valvalue val;
    val.decimal = value;
    values.push_back(std::make_unique<Value>(valtype::number, val));
    return *this;
}

List& List::put(float value) {
    return put((double)value);
}

List& List::put(bool value) {
    valvalue val;
    val.boolean = value;
    values.push_back(std::make_unique<Value>(valtype::boolean, val));
    return *this;
}

List& List::put(Map* value) {
    valvalue val;
    val.map = value;
    values.push_back(std::make_unique<Value>(valtype::map, val));
    return *this;
}

List& List::put(List* value) {
    valvalue val;
    val.list = value;
    values.push_back(std::make_unique<Value>(valtype::list, val));
    return *this;
}

List& List::putList() {
    List* arr = new List();
    put(arr);
    return *arr;
}

Map& List::putMap() {
    Map* map = new Map();
    put(map);
    return *map;
}

void List::remove(size_t index) {
    values.erase(values.begin() + index);
}

Map::~Map() {
}

void Map::str(std::string key, std::string& dst) const {
    dst = getStr(key, dst);
}

std::string Map::getStr(std::string key, const std::string& def) const {
    auto found = values.find(key);
    if (found == values.end())
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

double Map::getNum(std::string key, double def) const {
    auto found = values.find(key);
    if (found == values.end())
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

int64_t Map::getInt(std::string key, int64_t def) const {
    auto found = values.find(key);
    if (found == values.end())
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

bool Map::getBool(std::string key, bool def) const {
    auto found = values.find(key);
    if (found == values.end())
        return def;
    auto& val = found->second;
    switch (val->type) {
        case valtype::integer: return val->value.integer;
        case valtype::boolean: return val->value.boolean;
        default: throw std::runtime_error("type error");
    }
}

void Map::num(std::string key, double& dst) const {
    dst = getNum(key, dst);
}

void Map::num(std::string key, float& dst) const {
    dst = getNum(key, dst);
}

void Map::num(std::string key, ubyte& dst) const {
    dst = getInt(key, dst);
}

void Map::num(std::string key, int& dst) const {
    dst = getInt(key, dst);
}

void Map::num(std::string key, int64_t& dst) const {
    dst = getInt(key, dst);
}

void Map::num(std::string key, uint64_t& dst) const {
    dst = getInt(key, dst);
}

void Map::num(std::string key, uint& dst) const {
    dst = getInt(key, dst);
}

Map* Map::map(std::string key) const {
    auto found = values.find(key);
    if (found != values.end()) {
        auto& val = found->second;
        if (val->type != valtype::map)
            return nullptr;
        return val->value.map;
    }
    return nullptr;
}

List* Map::list(std::string key) const {
    auto found = values.find(key);
    if (found != values.end())
        return found->second->value.list;
    return nullptr;
}

void Map::flag(std::string key, bool& dst) const {
    dst = getBool(key, dst);
}

Map& Map::put(std::string key, uint value) {
    return put(key, (int64_t)value);
}

Map& Map::put(std::string key, int value) {
    return put(key, (int64_t)value);
}

Map& Map::put(std::string key, int64_t value) {
    valvalue val;
    val.integer = value;
    values[key] = std::make_unique<Value>(valtype::integer, val);
    return *this;
}

Map& Map::put(std::string key, uint64_t value) {
    return put(key, (int64_t)value);
}

Map& Map::put(std::string key, float value) {
    return put(key, (double)value);
}

Map& Map::put(std::string key, double value) {
    valvalue val;
    val.decimal = value;
    values[key] = std::make_unique<Value>(valtype::number, val);
    return *this;
}

Map& Map::put(std::string key, std::string value){
    valvalue val;
    val.str = new std::string(value);
    values[key] = std::make_unique<Value>(valtype::string, val);
    return *this;
}

Map& Map::put(std::string key, const char* value) {
    return put(key, std::string(value));
}

Map& Map::put(std::string key, Map* value){
    valvalue val;
    val.map = value;
    values[key] = std::make_unique<Value>(valtype::map, val);
    return *this;
}

Map& Map::put(std::string key, List* value){
    valvalue val;
    val.list = value;
    values[key] = std::make_unique<Value>(valtype::list, val);
    return *this;
}

Map& Map::put(std::string key, bool value){
    valvalue val;
    val.boolean = value;
    values[key] = std::make_unique<Value>(valtype::boolean, val);
    return *this;
}

List& Map::putList(std::string key) {
    List* arr = new List();
    put(key, arr);
    return *arr;
}

Map& Map::putMap(std::string key) {
    Map* obj = new Map();
    put(key, obj);
    return *obj;
}

bool Map::has(std::string key) {
    return values.find(key) != values.end();
}

Value::Value(valtype type, valvalue value) : type(type), value(value) {
}

Value::~Value() {
    switch (type) {
        case valtype::map: delete value.map; break;
        case valtype::list: delete value.list; break;
        case valtype::string: delete value.str; break;
        default:
            break;
    }
}
