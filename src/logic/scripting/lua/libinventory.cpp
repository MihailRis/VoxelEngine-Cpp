#include "lua_commons.hpp"
#include "api_lua.hpp"
#include "lua_util.hpp"
#include "../scripting.hpp"
#include "../../../content/Content.hpp"
#include "../../../world/Level.hpp"
#include "../../../items/ItemStack.hpp"
#include "../../../items/Inventories.hpp"
#include "../../../logic/BlocksController.hpp"

static void validate_itemid(itemid_t id) {
    if (id >= scripting::indices->countItemDefs()) {
        throw std::runtime_error("invalid item id");
    }
}

static std::shared_ptr<Inventory> get_inventory(int64_t id) {
    auto inv = scripting::level->inventories->get(id);
    if (inv == nullptr) {
        throw std::runtime_error("inventory not found: "+std::to_string(id));
    }
    return inv;
}

static std::shared_ptr<Inventory> get_inventory(int64_t id, int arg) {
    auto inv = scripting::level->inventories->get(id);
    if (inv == nullptr) {
        throw std::runtime_error("inventory not found: "+std::to_string(id)+
            " argument "+std::to_string(arg));
    }
    return inv;
}

static void validate_slotid(int slotid, Inventory* inv) {
    if (slotid < 0 || uint64_t(slotid) >= inv->size()) {
        throw std::runtime_error("slot index is out of range [0..inventory.size(invid)]");
    }
}

static int l_inventory_get(lua_State* L) {
    lua::luaint invid = lua_tointeger(L, 1);
    lua::luaint slotid = lua_tointeger(L, 2);
    auto inv = get_inventory(invid);
    validate_slotid(slotid, inv.get());
    const ItemStack& item = inv->getSlot(slotid);
    lua_pushinteger(L, item.getItemId());
    lua_pushinteger(L, item.getCount());
    return 2;
}

static int l_inventory_set(lua_State* L) {
    lua::luaint invid = lua_tointeger(L, 1);
    lua::luaint slotid = lua_tointeger(L, 2);
    lua::luaint itemid = lua_tointeger(L, 3);
    lua::luaint count = lua_tointeger(L, 4);
    validate_itemid(itemid);

    auto inv = get_inventory(invid);

    validate_slotid(slotid, inv.get());
    ItemStack& item = inv->getSlot(slotid);
    item.set(ItemStack(itemid, count));
    return 0;
}

static int l_inventory_size(lua_State* L) {
    lua::luaint invid = lua_tointeger(L, 1);
    auto inv = get_inventory(invid);
    lua_pushinteger(L, inv->size());
    return 1;
}

static int l_inventory_add(lua_State* L) {
    lua::luaint invid = lua_tointeger(L, 1);
    lua::luaint itemid = lua_tointeger(L, 2);
    lua::luaint count = lua_tointeger(L, 3);
    validate_itemid(itemid);

    auto inv = get_inventory(invid);
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

static int l_inventory_move(lua_State* L) {
    lua::luaint invAid = lua_tointeger(L, 1);
    lua::luaint slotAid = lua_tointeger(L, 2);
    auto invA = get_inventory(invAid, 1);
    validate_slotid(slotAid, invA.get());

    lua::luaint invBid = lua_tointeger(L, 3);
    lua::luaint slotBid = lua_isnil(L, 4) ? -1 : lua_tointeger(L, 4);
    auto invB = get_inventory(invBid, 3);
    auto& slot = invA->getSlot(slotAid);
    if (slotBid == -1) {
        invB->move(slot, scripting::content->getIndices());
    } else {
        invB->move(slot, scripting::content->getIndices(), slotBid, slotBid+1);
    }
    return 0;
}

const luaL_Reg inventorylib [] = {
    {"get", lua_wrap_errors<l_inventory_get>},
    {"set", lua_wrap_errors<l_inventory_set>},
    {"size", lua_wrap_errors<l_inventory_size>},
    {"add", lua_wrap_errors<l_inventory_add>},
    {"move", lua_wrap_errors<l_inventory_move>},
    {"get_block", lua_wrap_errors<l_inventory_get_block>},
    {"bind_block", lua_wrap_errors<l_inventory_bind_block>},
    {"unbind_block", lua_wrap_errors<l_inventory_unbind_block>},
    {"clone", lua_wrap_errors<l_inventory_clone>},
    {NULL, NULL}
};

