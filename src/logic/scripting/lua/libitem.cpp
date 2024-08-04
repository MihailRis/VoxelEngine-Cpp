#include "../../../content/Content.hpp"
#include "../../../items/ItemDef.hpp"
#include "api_lua.hpp"

using namespace scripting;

static const ItemDef* get_item_def(lua::State* L, int idx) {
    auto indices = content->getIndices();
    auto id = lua::tointeger(L, idx);
    return indices->items.get(id);
}

static int l_item_name(lua::State* L) {
    if (auto def = get_item_def(L, 1)) {
        return lua::pushstring(L, def->name);
    }
    return 0;
}

static int l_item_index(lua::State* L) {
    auto name = lua::require_string(L, 1);
    return lua::pushinteger(L, content->items.require(name).rt.id);
}

static int l_item_stack_size(lua::State* L) {
    if (auto def = get_item_def(L, 1)) {
        return lua::pushinteger(L, def->stackSize);
    }
    return 0;
}

static int l_item_defs_count(lua::State* L) {
    return lua::pushinteger(L, indices->items.count());
}

static int l_item_get_icon(lua::State* L) {
    if (auto def = get_item_def(L, 1)) {
        switch (def->iconType) {
            case item_icon_type::none:
                return 0;
            case item_icon_type::sprite:
                return lua::pushstring(L, def->icon);
            case item_icon_type::block:
                return lua::pushstring(L, "block-previews:" + def->icon);
        }
    }
    return 0;
}

const luaL_Reg itemlib[] = {
    {"index", lua::wrap<l_item_index>},
    {"name", lua::wrap<l_item_name>},
    {"stack_size", lua::wrap<l_item_stack_size>},
    {"defs_count", lua::wrap<l_item_defs_count>},
    {"icon", lua::wrap<l_item_get_icon>},
    {NULL, NULL}};
