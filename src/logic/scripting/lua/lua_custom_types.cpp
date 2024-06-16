#include "lua_custom_types.hpp"

#include "lua_util.hpp"

#include <sstream>

using namespace lua;


Bytearray::Bytearray(size_t capacity) 
  : buffer(std::make_unique<ubyte[]>(capacity)), capacity(capacity) {
}

Bytearray::~Bytearray() {
}

static int l_bytearray_meta_meta_call(lua::State* L) {
    auto size = tointeger(L, 2);
    if (size < 0) {
        throw std::runtime_error("size can not be less than 0");
    }
    return newuserdata<Bytearray>(L, static_cast<size_t>(size));
}

static int l_bytearray_meta_index(lua::State* L) {
    auto buffer = touserdata<Bytearray>(L, 1);
    auto index = tointeger(L, 2)-1;
    if (buffer == nullptr || static_cast<size_t>(index) > buffer->size()) {
        return 0;
    }
    return pushinteger(L, (*buffer)[index]);
}

static int l_bytearray_meta_newindex(lua::State* L) {
    auto buffer = touserdata<Bytearray>(L, 1);
    auto index = tointeger(L, 2)-1;
    if (buffer == nullptr || static_cast<size_t>(index) > buffer->size()) {
        return 0;
    }
    auto value = tointeger(L, 3);
    (*buffer)[index] = static_cast<ubyte>(value);
    return 0;
}

static int l_bytearray_meta_len(lua::State* L) {
    if (auto buffer = touserdata<Bytearray>(L, 1)) {
        return pushinteger(L, buffer->size());
    }
    return 0;
}

static int l_bytearray_meta_tostring(lua::State* L) {
    auto& buffer = *touserdata<Bytearray>(L, 1);
    if (buffer.size() > 128) {
        return pushstring(L, "bytearray["+std::to_string(buffer.size())+"]{...}");
    } else {
        std::stringstream ss;
        ss << "bytearray[" << std::to_string(buffer.size()) << "]{";
        for (size_t i = 0; i < buffer.size(); i++) {
            if (i > 0) {
                ss << " ";
            }
            ss << static_cast<uint>(buffer[i]);
        }
        ss << "}";
        return pushstring(L, ss.str());
    }
}

int Bytearray::createMetatable(lua::State* L) {
    createtable(L, 0, 5);
    pushcfunction(L, lua::wrap<l_bytearray_meta_index>);
    setfield(L, "__index");
    pushcfunction(L, lua::wrap<l_bytearray_meta_newindex>);
    setfield(L, "__newindex");
    pushcfunction(L, lua::wrap<l_bytearray_meta_len>);
    setfield(L, "__len");
    pushcfunction(L, lua::wrap<l_bytearray_meta_tostring>);
    setfield(L, "__tostring");

    createtable(L, 0, 1);
    pushcfunction(L, lua::wrap<l_bytearray_meta_meta_call>);
    setfield(L, "__call");
    setmetatable(L);
    return 1;
}
