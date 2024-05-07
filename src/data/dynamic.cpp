#include "dynamic.hpp"

using namespace dynamic;

List::~List() {
}

std::string List::str(size_t index) const {
    const auto& val = values[index];
    switch (static_cast<valtype>(val->value.index())) {
        case valtype::string: return std::get<std::string>(val->value);
        case valtype::boolean: return std::get<bool>(val->value) ? "true" : "false";
        case valtype::number: return std::to_string(std::get<double>(val->value));
        case valtype::integer: return std::to_string(std::get<int64_t>(val->value));
        default:
            throw std::runtime_error("type error");
    }
}

number_t List::num(size_t index) const {
    const auto& val = values[index];
    switch (static_cast<valtype>(val->value.index())) {
        case valtype::number: return std::get<number_t>(val->value);
        case valtype::integer: return std::get<integer_t>(val->value);
        case valtype::string: return std::stoll(std::get<std::string>(val->value));
        case valtype::boolean: return std::get<bool>(val->value);
        default:
            throw std::runtime_error("type error");
    }
}

integer_t List::integer(size_t index) const {
    const auto& val = values[index];
    switch (static_cast<valtype>(val->value.index())) {
        case valtype::number: return std::get<number_t>(val->value);
        case valtype::integer: return std::get<integer_t>(val->value);
        case valtype::string: return std::stoll(std::get<std::string>(val->value));
        case valtype::boolean: return std::get<bool>(val->value);
        default:
            throw std::runtime_error("type error");
    }
}

Map* List::map(size_t index) const {
    if (auto* val = std::get_if<Map*>(&values[index]->value)) {
        return *val;
    } else {
        throw std::runtime_error("type error");
    }
}

List* List::list(size_t index) const {
    if (auto* val = std::get_if<List*>(&values[index]->value)) {
        return *val;
    } else {
        throw std::runtime_error("type error");
    }
}

bool List::flag(size_t index) const {
    const auto& val = values[index];
    switch (static_cast<valtype>(val->value.index())) {
        case valtype::integer: return std::get<integer_t>(val->value);
        case valtype::boolean: return std::get<bool>(val->value);
        default:
            throw std::runtime_error("type error");
    }
}

Value* List::getValueWriteable(size_t index) const {
    if (index > values.size()) {
        throw std::runtime_error("index error");
    }
    return values.at(index).get();
}

List& List::put(std::unique_ptr<Value> value) {
    values.emplace_back(std::move(value));
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
    dst = get(key, dst);
}

std::string Map::get(const std::string& key, const std::string def) const {
    auto found = values.find(key);
    if (found == values.end())
        return def;
    auto& val = found->second;
    switch (static_cast<valtype>(val->value.index())) {
        case valtype::string: return std::get<std::string>(val->value);
        case valtype::boolean: return std::get<bool>(val->value) ? "true" : "false";
        case valtype::number: return std::to_string(std::get<number_t>(val->value));
        case valtype::integer: return std::to_string(std::get<integer_t>(val->value));
        default: throw std::runtime_error("type error");
    }
}

number_t Map::get(const std::string& key, double def) const {
    auto found = values.find(key);
    if (found == values.end())
        return def;
    auto& val = found->second;
    switch (static_cast<valtype>(val->value.index())) {
        case valtype::number: return std::get<number_t>(val->value);
        case valtype::integer: return std::get<integer_t>(val->value);
        case valtype::string: return std::stoull(std::get<std::string>(val->value));
        case valtype::boolean: return std::get<bool>(val->value);
        default: throw std::runtime_error("type error");
    }
}

integer_t Map::get(const std::string& key, integer_t def) const {
    auto found = values.find(key);
    if (found == values.end())
        return def;
    auto& val = found->second;
    switch (static_cast<valtype>(val->value.index())) {
        case valtype::number: return std::get<number_t>(val->value);
        case valtype::integer: return std::get<integer_t>(val->value);
        case valtype::string: return std::stoull(std::get<std::string>(val->value));
        case valtype::boolean: return std::get<bool>(val->value);
        default: throw std::runtime_error("type error");
    }
}

bool Map::get(const std::string& key, bool def) const {
    auto found = values.find(key);
    if (found == values.end())
        return def;
    auto& val = found->second;
    switch (static_cast<valtype>(val->value.index())) {
        case valtype::integer: return std::get<integer_t>(val->value);
        case valtype::boolean: return std::get<bool>(val->value);
        default: throw std::runtime_error("type error");
    }
}

void Map::num(std::string key, double& dst) const {
    dst = get(key, dst);
}

void Map::num(std::string key, float& dst) const {
    dst = get(key, static_cast<number_t>(dst));
}

void Map::num(std::string key, ubyte& dst) const {
    dst = get(key, static_cast<integer_t>(dst));
}

void Map::num(std::string key, int& dst) const {
    dst = get(key, static_cast<integer_t>(dst));
}

void Map::num(std::string key, int64_t& dst) const {
    dst = get(key, dst);
}

void Map::num(std::string key, uint64_t& dst) const {
    dst = get(key, static_cast<integer_t>(dst));
}

void Map::num(std::string key, uint& dst) const {
    dst = get(key, static_cast<integer_t>(dst));
}

Map* Map::map(std::string key) const {
    auto found = values.find(key);
    if (found != values.end()) {
        if (auto* val = std::get_if<Map*>(&found->second->value)) {
            return *val;
        }
    }
    return nullptr;
}

List* Map::list(std::string key) const {
    auto found = values.find(key);
    if (found != values.end())
        return std::get<List*>(found->second->value);
    return nullptr;
}

void Map::flag(std::string key, bool& dst) const {
    dst = get(key, dst);
}

Map& Map::put(std::string key, std::unique_ptr<Value> value) {
    values.emplace(key, value.release());
    return *this;
}

void Map::remove(const std::string& key) {
    values.erase(key);
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

bool Map::has(const std::string& key) const {
    return values.find(key) != values.end();
}

size_t Map::size() const {
    return values.size();
}

Value::Value(valvalue value) : value(value) {
}

Value::~Value() {
    switch (static_cast<valtype>(value.index())) {
        case valtype::map: delete std::get<Map*>(value); break;
        case valtype::list: delete std::get<List*>(value); break;
        default:
            break;
    }
}

std::unique_ptr<Value> Value::boolean(bool value) {
    return std::make_unique<Value>(value);
}

std::unique_ptr<Value> Value::of(number_u value) {
    if (std::holds_alternative<integer_t>(value)) {
        return std::make_unique<Value>(std::get<integer_t>(value));
    } else {
        return std::make_unique<Value>(std::get<number_t>(value));
    }
}

std::unique_ptr<Value> Value::of(const std::string& value) {
    return std::make_unique<Value>(value);
}

std::unique_ptr<Value> Value::of(std::unique_ptr<Map> value) {
    return std::make_unique<Value>(value.release());
}
