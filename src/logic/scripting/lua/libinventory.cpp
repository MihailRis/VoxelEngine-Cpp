#include <content/Content.hpp>
#include <items/Inventories.hpp>
#include <items/ItemStack.hpp>
#include <logic/BlocksController.hpp>
#include <world/Level.hpp>
#include "api_lua.hpp"

using namespace scripting;

static void validate_itemid(itemid_t id) {
    if (id >= indices->items.count()) {
        throw std::runtime_error("invalid item id");
    }
}

static std::shared_ptr<Inventory> get_inventory(int64_t id) {
    auto inv = level->inventories->get(id);
    if (inv == nullptr) {
        throw std::runtime_error("inventory not found: " + std::to_string(id));
    }
    return inv;
}

static std::shared_ptr<Inventory> get_inventory(int64_t id, int arg) {
    auto inv = level->inventories->get(id);
    if (inv == nullptr) {
        throw std::runtime_error(
            "inventory not found: " + std::to_string(id) + " argument " +
            std::to_string(arg)
        );
    }
    return inv;
}

static void validate_slotid(int slotid, Inventory* inv) {
    if (static_cast<size_t>(slotid) >= inv->size()) {
        throw std::runtime_error(
            "slot index is out of range [0..inventory.size(invid)]"
        );
    }
}

static int l_inventory_get(lua::State* L) {
    auto invid = lua::tointeger(L, 1);
    auto slotid = lua::tointeger(L, 2);
    auto inv = get_inventory(invid);
    validate_slotid(slotid, inv.get());
    const ItemStack& item = inv->getSlot(slotid);
    lua::pushinteger(L, item.getItemId());
    lua::pushinteger(L, item.getCount());
    return 2;
}

static int l_inventory_set(lua::State* L) {
    auto invid = lua::tointeger(L, 1);
    auto slotid = lua::tointeger(L, 2);
    auto itemid = lua::tointeger(L, 3);
    auto count = lua::tointeger(L, 4);
    validate_itemid(itemid);

    auto inv = get_inventory(invid);

    validate_slotid(slotid, inv.get());
    ItemStack& item = inv->getSlot(slotid);
    item.set(ItemStack(itemid, count));
    return 0;
}

static int l_inventory_size(lua::State* L) {
    auto invid = lua::tointeger(L, 1);
    auto inv = get_inventory(invid);
    return lua::pushinteger(L, inv->size());
}

static int l_inventory_add(lua::State* L) {
    auto invid = lua::tointeger(L, 1);
    auto itemid = lua::tointeger(L, 2);
    auto count = lua::tointeger(L, 3);
    validate_itemid(itemid);

    auto inv = get_inventory(invid);
    ItemStack item(itemid, count);
    inv->move(item, indices);
    return lua::pushinteger(L, item.getCount());
}

static int l_inventory_get_block(lua::State* L) {
    auto x = lua::tointeger(L, 1);
    auto y = lua::tointeger(L, 2);
    auto z = lua::tointeger(L, 3);
    int64_t id = blocks->createBlockInventory(x, y, z);
    return lua::pushinteger(L, id);
}

static int l_inventory_bind_block(lua::State* L) {
    auto id = lua::tointeger(L, 1);
    auto x = lua::tointeger(L, 2);
    auto y = lua::tointeger(L, 3);
    auto z = lua::tointeger(L, 4);
    blocks->bindInventory(id, x, y, z);
    return 0;
}

static int l_inventory_unbind_block(lua::State* L) {
    auto x = lua::tointeger(L, 1);
    auto y = lua::tointeger(L, 2);
    auto z = lua::tointeger(L, 3);
    blocks->unbindInventory(x, y, z);
    return 0;
}

static int l_inventory_clone(lua::State* L) {
    auto id = lua::tointeger(L, 1);
    auto clone = level->inventories->clone(id);
    if (clone == nullptr) {
        return lua::pushinteger(L, 0);
    }
    return lua::pushinteger(L, clone->getId());
}

static int l_inventory_move(lua::State* L) {
    auto invAid = lua::tointeger(L, 1);
    auto slotAid = lua::tointeger(L, 2);
    auto invA = get_inventory(invAid, 1);
    validate_slotid(slotAid, invA.get());

    auto invBid = lua::tointeger(L, 3);
    auto slotBid = lua::isnil(L, 4) ? -1 : lua::tointeger(L, 4);
    auto invB = get_inventory(invBid, 3);
    auto& slot = invA->getSlot(slotAid);
    if (slotBid == -1) {
        invB->move(slot, content->getIndices());
    } else {
        invB->move(slot, content->getIndices(), slotBid, slotBid + 1);
    }
    return 0;
}

const luaL_Reg inventorylib[] = {
    {"get", lua::wrap<l_inventory_get>},
    {"set", lua::wrap<l_inventory_set>},
    {"size", lua::wrap<l_inventory_size>},
    {"add", lua::wrap<l_inventory_add>},
    {"move", lua::wrap<l_inventory_move>},
    {"get_block", lua::wrap<l_inventory_get_block>},
    {"bind_block", lua::wrap<l_inventory_bind_block>},
    {"unbind_block", lua::wrap<l_inventory_unbind_block>},
    {"clone", lua::wrap<l_inventory_clone>},
    {NULL, NULL}};
