#include "content/Content.hpp"
#include "items/ItemDef.hpp"
#include "api_lua.hpp"

using namespace scripting;

static const ItemDef* get_item_def(lua::State* L, int idx) {
    auto indices = content->getIndices();
    auto id = lua::tointeger(L, idx);
    return indices->items.get(id);
}

static int l_name(lua::State* L) {
    if (auto def = get_item_def(L, 1)) {
        return lua::pushstring(L, def->name);
    }
    return 0;
}

static int l_index(lua::State* L) {
    auto name = lua::require_string(L, 1);
    return lua::pushinteger(L, content->items.require(name).rt.id);
}

static int l_stack_size(lua::State* L) {
    if (auto def = get_item_def(L, 1)) {
        return lua::pushinteger(L, def->stackSize);
    }
    return 0;
}

static int l_defs_count(lua::State* L) {
    return lua::pushinteger(L, indices->items.count());
}

static int l_get_icon(lua::State* L) {
    if (auto def = get_item_def(L, 1)) {
        switch (def->iconType) {
            case ItemIconType::NONE:
                return 0;
            case ItemIconType::SPRITE:
                return lua::pushstring(L, def->icon);
            case ItemIconType::BLOCK:
                return lua::pushstring(L, "block-previews:" + def->icon);
        }
    }
    return 0;
}

static int l_caption(lua::State* L) {
    if (auto def = get_item_def(L, 1)) {
        return lua::pushstring(L, def->caption);
    }
    return 0;
}

static int l_get_placing_block(lua::State* L) {
    if (auto def = get_item_def(L, 1)) {
        return lua::pushinteger(L, def->rt.placingBlock);
    }
    return 0;
}

const luaL_Reg itemlib[] = {
    {"index", lua::wrap<l_index>},
    {"name", lua::wrap<l_name>},
    {"stack_size", lua::wrap<l_stack_size>},
    {"defs_count", lua::wrap<l_defs_count>},
    {"icon", lua::wrap<l_get_icon>},
    {"caption", lua::wrap<l_caption>},
    {"get_placing_block", lua::wrap<l_get_placing_block>},
    {NULL, NULL}};
