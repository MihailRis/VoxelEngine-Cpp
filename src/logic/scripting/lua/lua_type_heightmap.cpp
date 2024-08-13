#include "lua_custom_types.hpp"

#include <cstring>
#include <sstream>
#include <iomanip>

#include "lua_util.hpp"

using namespace lua;

Heightmap::Heightmap(uint width, uint height) : width(width), height(height) {
    buffer.resize(width*height);
}

Heightmap::~Heightmap() {
}

static int l_meta_meta_call(lua::State* L) {
    auto width = tointeger(L, 2);
    auto height = tointeger(L, 3);
    if (width <= 0 || height <= 0) {
        throw std::runtime_error("width and height must be greather than 0");
    }
    return newuserdata<Heightmap>(
        L, static_cast<uint>(width), static_cast<uint>(height)
    );
}

static int l_meta_index(lua::State* L) {
    auto map = touserdata<Heightmap>(L, 1);
    if (map == nullptr) {
        return 0;
    }
    if (isstring(L, 2)) {
        auto fieldname = tostring(L, 2);
        if (!std::strcmp(fieldname, "width")) {
            return pushinteger(L, map->getWidth());
        } else if (!std::strcmp(fieldname, "height")) {
            return pushinteger(L, map->getHeight());
        }
    }
    return 0;
}

static int l_meta_tostring(lua::State* L) {
    auto map = touserdata<Heightmap>(L, 1);
    if (map == nullptr) {
        return 0;
    }

    std::stringstream stream;
    stream << std::hex << reinterpret_cast<ptrdiff_t>(map);
    auto ptrstr = stream.str();

    return pushstring(
        L, "Heightmap(" + std::to_string(map->getWidth()) + 
           "*" + std::to_string(map->getHeight()) + " at 0x" + ptrstr + ")"
    );
}

int Heightmap::createMetatable(lua::State* L) {
    createtable(L, 0, 2);
    pushcfunction(L, lua::wrap<l_meta_tostring>);
    setfield(L, "__tostring");
    pushcfunction(L, lua::wrap<l_meta_index>);
    setfield(L, "__index");

    createtable(L, 0, 1);
    pushcfunction(L, lua::wrap<l_meta_meta_call>);
    setfield(L, "__call");
    setmetatable(L);
    return 1;
}
