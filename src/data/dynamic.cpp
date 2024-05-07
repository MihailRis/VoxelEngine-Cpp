#include "dynamic.hpp"

using namespace dynamic;

std::string List::str(size_t index) const {
    const auto& value = values[index];
    switch (static_cast<valtype>(value.index())) {
        case valtype::string: return std::get<std::string>(value);
        case valtype::boolean: return std::get<bool>(value) ? "true" : "false";
        case valtype::number: return std::to_string(std::get<double>(value));
        case valtype::integer: return std::to_string(std::get<int64_t>(value));
        default:
            throw std::runtime_error("type error");
    }
}

number_t List::num(size_t index) const {
    const auto& value = values[index];
    switch (static_cast<valtype>(value.index())) {
        case valtype::number: return std::get<number_t>(value);
        case valtype::integer: return std::get<integer_t>(value);
        case valtype::string: return std::stoll(std::get<std::string>(value));
        case valtype::boolean: return std::get<bool>(value);
        default:
            throw std::runtime_error("type error");
    }
}

integer_t List::integer(size_t index) const {
    const auto& value = values[index];
    switch (static_cast<valtype>(value.index())) {
        case valtype::number: return std::get<number_t>(value);
        case valtype::integer: return std::get<integer_t>(value);
        case valtype::string: return std::stoll(std::get<std::string>(value));
        case valtype::boolean: return std::get<bool>(value);
        default:
            throw std::runtime_error("type error");
    }
}

Map* List::map(size_t index) const {
    if (auto* val = std::get_if<Map_sptr>(&values[index])) {
        return val->get();
    } else {
        throw std::runtime_error("type error");
    }
}

List* List::list(size_t index) const {
    if (auto* val = std::get_if<List_sptr>(&values[index])) {
        return val->get();
    } else {
        throw std::runtime_error("type error");
    }
}

bool List::flag(size_t index) const {
    const auto& value = values[index];
    switch (static_cast<valtype>(value.index())) {
        case valtype::integer: return std::get<integer_t>(value);
        case valtype::boolean: return std::get<bool>(value);
        default:
            throw std::runtime_error("type error");
    }
}

Value* List::getValueWriteable(size_t index) {
    if (index > values.size()) {
        throw std::runtime_error("index error");
    }
    return &values.at(index);
}

List& List::put(const Value& value) {
    values.emplace_back(value);
    return *this;
}

List& List::putList() {
    auto arr = std::make_shared<List>();
    put(arr);
    return *arr;
}

Map& List::putMap() {
    auto map = std::make_shared<Map>();
    put(map);
    return *map;
}

void List::remove(size_t index) {
    values.erase(values.begin() + index);
}

void Map::str(const std::string& key, std::string& dst) const {
    dst = get(key, dst);
}

std::string Map::get(const std::string& key, const std::string def) const {
    auto found = values.find(key);
    if (found == values.end())
        return def;
    auto& value = found->second;
    switch (static_cast<valtype>(value.index())) {
        case valtype::string: return std::get<std::string>(value);
        case valtype::boolean: return std::get<bool>(value) ? "true" : "false";
        case valtype::number: return std::to_string(std::get<number_t>(value));
        case valtype::integer: return std::to_string(std::get<integer_t>(value));
        default: throw std::runtime_error("type error");
    }
}

number_t Map::get(const std::string& key, double def) const {
    auto found = values.find(key);
    if (found == values.end())
        return def;
    auto& value = found->second;
    switch (static_cast<valtype>(value.index())) {
        case valtype::number: return std::get<number_t>(value);
        case valtype::integer: return std::get<integer_t>(value);
        case valtype::string: return std::stoull(std::get<std::string>(value));
        case valtype::boolean: return std::get<bool>(value);
        default: throw std::runtime_error("type error");
    }
}

integer_t Map::get(const std::string& key, integer_t def) const {
    auto found = values.find(key);
    if (found == values.end())
        return def;
    auto& value = found->second;
    switch (static_cast<valtype>(value.index())) {
        case valtype::number: return std::get<number_t>(value);
        case valtype::integer: return std::get<integer_t>(value);
        case valtype::string: return std::stoull(std::get<std::string>(value));
        case valtype::boolean: return std::get<bool>(value);
        default: throw std::runtime_error("type error");
    }
}

bool Map::get(const std::string& key, bool def) const {
    auto found = values.find(key);
    if (found == values.end())
        return def;
    auto& value = found->second;
    switch (static_cast<valtype>(value.index())) {
        case valtype::integer: return std::get<integer_t>(value);
        case valtype::boolean: return std::get<bool>(value);
        default: throw std::runtime_error("type error");
    }
}

void Map::num(const std::string& key, double& dst) const {
    dst = get(key, dst);
}

void Map::num(const std::string& key, float& dst) const {
    dst = get(key, static_cast<number_t>(dst));
}

void Map::num(const std::string& key, ubyte& dst) const {
    dst = get(key, static_cast<integer_t>(dst));
}

void Map::num(const std::string& key, int& dst) const {
    dst = get(key, static_cast<integer_t>(dst));
}

void Map::num(const std::string& key, int64_t& dst) const {
    dst = get(key, dst);
}

void Map::num(const std::string& key, uint64_t& dst) const {
    dst = get(key, static_cast<integer_t>(dst));
}

void Map::num(const std::string& key, uint& dst) const {
    dst = get(key, static_cast<integer_t>(dst));
}

Map* Map::map(const std::string& key) const {
    auto found = values.find(key);
    if (found != values.end()) {
        if (auto* val = std::get_if<Map_sptr>(&found->second)) {
            return val->get();
        }
    }
    return nullptr;
}

List* Map::list(const std::string& key) const {
    auto found = values.find(key);
    if (found != values.end())
        return std::get<List_sptr>(found->second).get();
    return nullptr;
}

void Map::flag(const std::string& key, bool& dst) const {
    dst = get(key, dst);
}

Map& Map::put(std::string key, const Value& value) {
    values.emplace(key, value);
    return *this;
}

void Map::remove(const std::string& key) {
    values.erase(key);
}

List& Map::putList(std::string key) {
    auto arr = std::make_shared<List>();
    put(key, arr);
    return *arr;
}

Map& Map::putMap(std::string key) {
    auto obj = std::make_shared<Map>();
    put(key, obj);
    return *obj;
}

bool Map::has(const std::string& key) const {
    return values.find(key) != values.end();
}

size_t Map::size() const {
    return values.size();
}
