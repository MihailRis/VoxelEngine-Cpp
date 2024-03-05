#include "lua_commons.h"
#include "api_lua.h"

#include "lua_util.h"
#include "../scripting.h"
#include "../../../content/Content.h"
#include "../../../world/Level.h"
#include "../../../items/ItemStack.h"
#include "../../../items/Inventories.h"
#include "../../../logic/BlocksController.h"

static void validate_itemid(lua_State* L, itemid_t id) {
    if (id >= scripting::indices->countItemDefs()) {
        luaL_error(L, "invalid item id");
    }
}

static int l_inventory_get(lua_State* L) {
    lua::luaint invid = lua_tointeger(L, 1);
    lua::luaint slotid = lua_tointeger(L, 2);
    auto inv = scripting::level->inventories->get(invid);
    if (inv == nullptr) {
        luaL_error(L, "inventory does not exists in runtime: %d", invid);
    }
    if (slotid < 0 || uint64_t(slotid) >= inv->size()) {
        luaL_error(L, "slot index is out of range [0, inventory.size(invid)]");
    }
    ItemStack& item = inv->getSlot(slotid);
    lua_pushinteger(L, item.getItemId());
    lua_pushinteger(L, item.getCount());
    return 2;
}

static int l_inventory_set(lua_State* L) {
    lua::luaint invid = lua_tointeger(L, 1);
    lua::luaint slotid = lua_tointeger(L, 2);
    lua::luaint itemid = lua_tointeger(L, 3);
    lua::luaint count = lua_tointeger(L, 4);
    validate_itemid(L, itemid);

    auto inv = scripting::level->inventories->get(invid);
    if (inv == nullptr) {
        luaL_error(L, "inventory does not exists in runtime: %d", invid);
    }
    if (slotid < 0 || uint64_t(slotid) >= inv->size()) {
        luaL_error(L, "slot index is out of range [0, inventory.size(invid)]");
    }
    ItemStack& item = inv->getSlot(slotid);
    item.set(ItemStack(itemid, count));
    return 0;
}

static int l_inventory_size(lua_State* L) {
    lua::luaint invid = lua_tointeger(L, 1);
    auto inv = scripting::level->inventories->get(invid);
    if (inv == nullptr) {
        luaL_error(L, "inventory does not exists in runtime: %d", invid);
    }
    lua_pushinteger(L, inv->size());
    return 1;
}

static int l_inventory_add(lua_State* L) {
    lua::luaint invid = lua_tointeger(L, 1);
    lua::luaint itemid = lua_tointeger(L, 2);
    lua::luaint count = lua_tointeger(L, 3);
    validate_itemid(L, itemid);

    auto inv = scripting::level->inventories->get(invid);
    if (inv == nullptr) {
        luaL_error(L, "inventory does not exists in runtime: %d", invid);
    }
    ItemStack item(itemid, count);
    inv->move(item, scripting::indices);
    lua_pushinteger(L, item.getCount());
    return 1;
}

static int l_inventory_get_block(lua_State* L) {
    lua::luaint x = lua_tointeger(L, 1);
    lua::luaint y = lua_tointeger(L, 2);
    lua::luaint z = lua_tointeger(L, 3);
    int64_t id = scripting::blocks->createBlockInventory(x, y, z);
    lua_pushinteger(L, id);
    return 1;
}

static int l_inventory_bind_block(lua_State* L) {
    lua::luaint id = lua_tointeger(L, 1);
    lua::luaint x = lua_tointeger(L, 2);
    lua::luaint y = lua_tointeger(L, 3);
    lua::luaint z = lua_tointeger(L, 4);
    scripting::blocks->bindInventory(id, x, y, z);
    return 0;
}

static int l_inventory_unbind_block(lua_State* L) {
    lua::luaint x = lua_tointeger(L, 1);
    lua::luaint y = lua_tointeger(L, 2);
    lua::luaint z = lua_tointeger(L, 3);
    scripting::blocks->unbindInventory(x, y, z);
    return 0;
}

static int l_inventory_clone(lua_State* L) {
    lua::luaint id = lua_tointeger(L, 1);
    auto clone = scripting::level->inventories->clone(id);
    if (clone == nullptr) {
        lua_pushinteger(L, 0);
        return 1;
    }
    lua_pushinteger(L, clone->getId());
    return 1;
}

const luaL_Reg inventorylib [] = {
    {"get", lua_wrap_errors<l_inventory_get>},
    {"set", lua_wrap_errors<l_inventory_set>},
    {"size", lua_wrap_errors<l_inventory_size>},
    {"add", lua_wrap_errors<l_inventory_add>},
    {"get_block", lua_wrap_errors<l_inventory_get_block>},
    {"bind_block", lua_wrap_errors<l_inventory_bind_block>},
    {"unbind_block", lua_wrap_errors<l_inventory_unbind_block>},
    {"clone", lua_wrap_errors<l_inventory_clone>},
    {NULL, NULL}
};

