#include "lua_commons.hpp"
#include "api_lua.hpp"
#include "../scripting.hpp"
#include "../../../content/Content.hpp"
#include "../../../items/ItemDef.hpp"

static int l_item_name(lua_State* L) {
    auto indices = scripting::content->getIndices();
    lua::luaint id = lua_tointeger(L, 1);
    if (id < 0 || size_t(id) >= indices->countItemDefs()) {
        return 0;
    }
    auto def = indices->getItemDef(id);
    lua_pushstring(L, def->name.c_str());
    return 1;
}

static int l_item_index(lua_State* L) {
    auto name = lua_tostring(L, 1);
    lua_pushinteger(L, scripting::content->requireItem(name).rt.id);
    return 1;
}

static int l_item_stack_size(lua_State* L) {
    auto indices = scripting::content->getIndices();
    lua::luaint id = lua_tointeger(L, 1);
    if (id < 0 || size_t(id) >= indices->countItemDefs()) {
        return 0;
    }
    auto def = indices->getItemDef(id);
    lua_pushinteger(L, def->stackSize);
    return 1;
}

static int l_item_defs_count(lua_State* L) {
    lua_pushinteger(L, scripting::indices->countItemDefs());
    return 1;
}

const luaL_Reg itemlib [] = {
    {"index", lua_wrap_errors<l_item_index>},
    {"name", lua_wrap_errors<l_item_name>},
    {"stack_size", lua_wrap_errors<l_item_stack_size>},
    {"defs_count", lua_wrap_errors<l_item_defs_count>},
    {NULL, NULL}
};
