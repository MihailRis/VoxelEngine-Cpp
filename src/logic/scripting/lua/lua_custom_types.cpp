#include "lua_custom_types.hpp"

#include <sstream>

#include "lua_util.hpp"

using namespace lua;

Bytearray::Bytearray(size_t capacity) : buffer(capacity) {
    buffer.resize(capacity);
}

Bytearray::Bytearray(std::vector<ubyte> buffer) : buffer(std::move(buffer)) {
}

Bytearray::~Bytearray() {
}

static int l_bytearray_append(lua::State* L) {
    if (auto buffer = touserdata<Bytearray>(L, 1)) {
        auto value = tointeger(L, 2);
        buffer->data().push_back(static_cast<ubyte>(value));
    }
    return 0;
}

static int l_bytearray_insert(lua::State* L) {
    auto buffer = touserdata<Bytearray>(L, 1);
    if (buffer == nullptr) {
        return 0;
    }
    auto& data = buffer->data();
    auto index = tointeger(L, 2) - 1;
    if (static_cast<size_t>(index) > data.size()) {
        return 0;
    }
    auto value = tointeger(L, 3);
    data.insert(data.begin() + index, static_cast<ubyte>(value));
    return 0;
}

static int l_bytearray_remove(lua::State* L) {
    auto buffer = touserdata<Bytearray>(L, 1);
    if (buffer == nullptr) {
        return 0;
    }
    auto& data = buffer->data();
    auto index = tointeger(L, 2) - 1;
    if (static_cast<size_t>(index) > data.size()) {
        return 0;
    }
    data.erase(data.begin() + index);
    return 0;
}

static std::unordered_map<std::string, lua_CFunction> bytearray_methods {
    {"append", lua::wrap<l_bytearray_append>},
    {"insert", lua::wrap<l_bytearray_insert>},
    {"remove", lua::wrap<l_bytearray_remove>},
};

static int l_bytearray_meta_meta_call(lua::State* L) {
    if (lua_istable(L, 2)) {
        size_t len = objlen(L, 2);
        std::vector<ubyte> buffer(len);
        buffer.resize(len);
        for (size_t i = 0; i < len; i++) {
            rawgeti(L, i + 1);
            buffer[i] = static_cast<ubyte>(tointeger(L, -1));
            pop(L);
        }
        return newuserdata<Bytearray>(L, std::move(buffer));
    }
    auto size = tointeger(L, 2);
    if (size < 0) {
        throw std::runtime_error("size can not be less than 0");
    }
    return newuserdata<Bytearray>(L, static_cast<size_t>(size));
}

static int l_bytearray_meta_index(lua::State* L) {
    auto buffer = touserdata<Bytearray>(L, 1);
    if (buffer == nullptr) {
        return 0;
    }
    auto& data = buffer->data();
    if (isstring(L, 2)) {
        auto found = bytearray_methods.find(tostring(L, 2));
        if (found != bytearray_methods.end()) {
            return pushcfunction(L, found->second);
        }
    }
    auto index = tointeger(L, 2) - 1;
    if (static_cast<size_t>(index) > data.size()) {
        return 0;
    }
    return pushinteger(L, data[index]);
}

static int l_bytearray_meta_newindex(lua::State* L) {
    auto buffer = touserdata<Bytearray>(L, 1);
    if (buffer == nullptr) {
        return 0;
    }
    auto& data = buffer->data();
    auto index = static_cast<size_t>(tointeger(L, 2) - 1);
    auto value = tointeger(L, 3);
    if (index >= data.size()) {
        if (index == data.size()) {
            data.push_back(static_cast<ubyte>(value));
        }
        return 0;
    }
    data[index] = static_cast<ubyte>(value);
    return 0;
}

static int l_bytearray_meta_len(lua::State* L) {
    if (auto buffer = touserdata<Bytearray>(L, 1)) {
        return pushinteger(L, buffer->data().size());
    }
    return 0;
}

static int l_bytearray_meta_tostring(lua::State* L) {
    auto buffer = touserdata<Bytearray>(L, 1);
    if (buffer == nullptr) {
        return 0;
    }
    auto& data = buffer->data();
    if (data.size() > 512) {
        return pushstring(
            L, "bytearray[" + std::to_string(data.size()) + "]{...}"
        );
    } else {
        std::stringstream ss;
        ss << "bytearray[" << std::to_string(data.size()) << "]{";
        for (size_t i = 0; i < data.size(); i++) {
            if (i > 0) {
                ss << " ";
            }
            ss << static_cast<uint>(data[i]);
        }
        ss << "}";
        return pushstring(L, ss.str());
    }
}

static int l_bytearray_meta_add(lua::State* L) {
    auto bufferA = touserdata<Bytearray>(L, 1);
    auto bufferB = touserdata<Bytearray>(L, 2);
    if (bufferA == nullptr || bufferB == nullptr) {
        return 0;
    }
    auto& dataA = bufferA->data();
    auto& dataB = bufferB->data();

    std::vector<ubyte> ab;
    ab.reserve(dataA.size() + dataB.size());
    ab.insert(ab.end(), dataA.begin(), dataA.end());
    ab.insert(ab.end(), dataB.begin(), dataB.end());
    return newuserdata<Bytearray>(L, std::move(ab));
}

int Bytearray::createMetatable(lua::State* L) {
    createtable(L, 0, 6);
    pushcfunction(L, lua::wrap<l_bytearray_meta_index>);
    setfield(L, "__index");
    pushcfunction(L, lua::wrap<l_bytearray_meta_newindex>);
    setfield(L, "__newindex");
    pushcfunction(L, lua::wrap<l_bytearray_meta_len>);
    setfield(L, "__len");
    pushcfunction(L, lua::wrap<l_bytearray_meta_tostring>);
    setfield(L, "__tostring");
    pushcfunction(L, lua::wrap<l_bytearray_meta_add>);
    setfield(L, "__add");

    createtable(L, 0, 1);
    pushcfunction(L, lua::wrap<l_bytearray_meta_meta_call>);
    setfield(L, "__call");
    setmetatable(L);
    return 1;
}
