#include "api_lua.hpp"

#include "../../../content/Content.hpp"
#include "../../../items/ItemDef.hpp"

using namespace scripting;

static int l_item_name(lua::State* L) {
    auto indices = content->getIndices();
    auto id = lua::tointeger(L, 1);
    if (static_cast<size_t>(id) >= indices->items.count()) {
        return 0;
    }
    auto def = indices->items.get(id);
    return lua::pushstring(L, def->name);
}

static int l_item_index(lua::State* L) {
    auto name = lua::require_string(L, 1);
    return lua::pushinteger(L, content->items.require(name).rt.id);
}

static int l_item_stack_size(lua::State* L) {
    auto indices = content->getIndices();
    auto id = lua::tointeger(L, 1);
    if (static_cast<size_t>(id) >= indices->items.count()) {
        return 0;
    }
    auto def = indices->items.get(id);
    return lua::pushinteger(L, def->stackSize);
}

static int l_item_defs_count(lua::State* L) {
    return lua::pushinteger(L, indices->items.count());
}

const luaL_Reg itemlib [] = {
    {"index", lua::wrap<l_item_index>},
    {"name", lua::wrap<l_item_name>},
    {"stack_size", lua::wrap<l_item_stack_size>},
    {"defs_count", lua::wrap<l_item_defs_count>},
    {NULL, NULL}
};
