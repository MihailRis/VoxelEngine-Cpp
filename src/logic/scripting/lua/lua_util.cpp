#include "lua_util.hpp"

#include "../../../debug/Logger.hpp"
#include "../../../util/stringutil.hpp"

static debug::Logger logger("lua");

int lua::pushvalue(lua_State* L, const dynamic::Value& value) {
    using namespace dynamic;

    if (auto* flag = std::get_if<bool>(&value)) {
        lua_pushboolean(L, *flag);
    } else if (auto* num = std::get_if<integer_t>(&value)) {
        lua_pushinteger(L, *num);
    } else if (auto* num = std::get_if<number_t>(&value)) {
        lua_pushnumber(L, *num);
    } else if (auto* str = std::get_if<std::string>(&value)) {
        lua_pushstring(L, str->c_str());
    } else if (auto listptr = std::get_if<List_sptr>(&value)) {
        auto list = *listptr;
        lua_createtable(L, list->size(), 0);
        for (size_t i = 0; i < list->size(); i++) {
            pushvalue(L, list->get(i));
            lua_rawseti(L, -2, i+1);
        }
    } else if (auto mapptr = std::get_if<Map_sptr>(&value)) {
        auto map = *mapptr;
        lua_createtable(L, 0, map->size());
        for (auto& entry : map->values) {
            pushvalue(L, entry.second);
            lua_setfield(L, -2, entry.first.c_str());
        }
    } else {
        lua_pushnil(L);
    }
    return 1;
}

std::wstring lua::require_wstring(lua_State* L, int idx) {
    return util::str2wstr_utf8(lua::require_string(L, idx));
}

int lua::pushwstring(lua_State* L, const std::wstring& str) {
    return lua::pushstring(L, util::wstr2str_utf8(str));
}

dynamic::Value lua::tovalue(lua_State* L, int idx) {
    using namespace dynamic;
    auto type = lua_type(L, idx);
    switch (type) {
        case LUA_TNIL:
        case LUA_TNONE:
            return dynamic::NONE;
        case LUA_TBOOLEAN:
            return lua_toboolean(L, idx) == 1;
        case LUA_TNUMBER: {
            auto number = lua_tonumber(L, idx);
            auto integer = lua_tointeger(L, idx);
            if (number == (lua_Number)integer) {
                return integer;
            } else {
                return number;
            }
        }
        case LUA_TSTRING:
            return std::string(lua_tostring(L, idx));
        case LUA_TTABLE: {
            int len = lua_objlen(L, idx);
            if (len) {
                // array
                auto list = create_list();
                for (int i = 1; i <= len; i++) {
                    lua_rawgeti(L, idx, i);
                    list->put(tovalue(L, -1));
                    lua_pop(L, 1);
                }
                return list;
            } else {
                // table
                auto map = create_map();
                lua_pushvalue(L, idx);
                lua_pushnil(L);
                while (lua_next(L, -2)) {
                    lua_pushvalue(L, -2);
                    auto key = lua_tostring(L, -1);
                    map->put(key, tovalue(L, -2));
                    lua_pop(L, 2);
                }
                lua_pop(L, 1);
                return map;
            }
        }
        default:
            throw std::runtime_error(
                "lua type "+std::string(luaL_typename(L, type))+" is not supported"
            );
    }
}

void lua::logError(const std::string& text) {
    logger.error() << text;
}

int lua::call(lua_State* L, int argc, int nresults) {
    if (lua_pcall(L, argc, nresults, 0)) {
        throw luaerror(lua_tostring(L, -1));
    }
    return 1;
}

int lua::callNoThrow(lua_State* L, int argc) {
    if (lua_pcall(L, argc, LUA_MULTRET, 0)) {
        logError(lua_tostring(L, -1));
        return 0;
    }
    return 1;
}
