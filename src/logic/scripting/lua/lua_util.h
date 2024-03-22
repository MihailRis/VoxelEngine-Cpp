#ifndef LOGIC_SCRIPTING_LUA_UTIL_H_
#define LOGIC_SCRIPTING_LUA_UTIL_H_

#ifdef __linux__ 
#include <luajit-2.1/luaconf.h>
#include <luajit-2.1/lua.hpp>
#else
#include <lua.hpp>
#endif
#include <glm/glm.hpp>
#include "LuaState.h"

namespace lua {
    inline int pushivec3(lua_State* L, luaint x, luaint y, luaint z) {
        lua_pushinteger(L, x);
        lua_pushinteger(L, y);
        lua_pushinteger(L, z);
        return 3;
    }

    inline int pushvec3(lua_State* L, glm::vec3 vec) {
        lua_pushnumber(L, vec.x);
        lua_pushnumber(L, vec.y);
        lua_pushnumber(L, vec.z);
        return 3;
    }

    inline int pushvec4(lua_State* L, glm::vec4 vec) {
        lua_pushnumber(L, vec.x);
        lua_pushnumber(L, vec.y);
        lua_pushnumber(L, vec.z);
        lua_pushnumber(L, vec.w);
        return 4;
    }

    inline int pushvec2_arr(lua_State* L, glm::vec2 vec) {
        lua_createtable(L, 2, 0);
        lua_getglobal(L, "vec2_mt");
        lua_setmetatable(L, -2);

        lua_pushnumber(L, vec.x);
        lua_rawseti(L, -2, 1);
        lua_pushnumber(L, vec.y);
        lua_rawseti(L, -2, 2);
        return 1;
    }

    inline int pushvec3_arr(lua_State* L, glm::vec3 vec) {
        lua_createtable(L, 3, 0);
        lua_getglobal(L, "vec3_mt");
        lua_setmetatable(L, -2);

        lua_pushnumber(L, vec.x);
        lua_rawseti(L, -2, 1);
        lua_pushnumber(L, vec.y);
        lua_rawseti(L, -2, 2);
        lua_pushnumber(L, vec.z);
        lua_rawseti(L, -2, 3);
        return 1;
    }

    inline int pushvec4_arr(lua_State* L, glm::vec4 vec) {
        lua_createtable(L, 4, 0);
        lua_getglobal(L, "vec4_mt");
        lua_setmetatable(L, -2);

        lua_pushnumber(L, vec.x);
        lua_rawseti(L, -2, 1);
        lua_pushnumber(L, vec.y);
        lua_rawseti(L, -2, 2);
        lua_pushnumber(L, vec.z);
        lua_rawseti(L, -2, 3);
        lua_pushnumber(L, vec.w);
        lua_rawseti(L, -2, 4);
        return 1;
    }

    inline int pushcolor_arr(lua_State* L, glm::vec4 vec) {
        lua_createtable(L, 4, 0);
        lua_getglobal(L, "color_mt");
        lua_setmetatable(L, -2);

        lua_pushinteger(L, vec.x*255);
        lua_rawseti(L, -2, 1);
        lua_pushinteger(L, vec.y*255);
        lua_rawseti(L, -2, 2);
        lua_pushinteger(L, vec.z*255);
        lua_rawseti(L, -2, 3);
        lua_pushinteger(L, vec.w*255);
        lua_rawseti(L, -2, 4);
        return 1;
    }

    inline glm::vec2 tovec2(lua_State* L, int idx) {
        lua_pushvalue(L, idx);
        lua_rawgeti(L, -1, 1);
        lua::luanumber x = lua_tonumber(L, -1); lua_pop(L, 1);
        lua_rawgeti(L, -1, 2);
        lua::luanumber y = lua_tonumber(L, -1); lua_pop(L, 1);
        lua_pop(L, 1);
        return glm::vec2(x, y);
    }

    inline glm::vec4 tocolor(lua_State* L, int idx) {
        lua_pushvalue(L, idx);
        if (!lua_istable(L, -1)) {
            luaL_error(L, "RGBA array required");
        }
        lua_rawgeti(L, -1, 1);
        lua::luanumber r = lua_tonumber(L, -1); lua_pop(L, 1);
        lua_rawgeti(L, -1, 2);
        lua::luanumber g = lua_tonumber(L, -1); lua_pop(L, 1);
        lua_rawgeti(L, -1, 3);
        lua::luanumber b = lua_tonumber(L, -1); lua_pop(L, 1);
        lua_rawgeti(L, -1, 4);
        lua::luanumber a = lua_tonumber(L, -1); lua_pop(L, 1);
        lua_pop(L, 1);
        return glm::vec4(r/255, g/255, b/255, a/255);
    }
}

#endif // LOGIC_SCRIPTING_LUA_UTIL_H_
