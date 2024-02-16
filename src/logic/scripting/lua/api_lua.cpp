#include "api_lua.h"

#include "../scripting.h"
#include "lua_util.h"

#include <glm/glm.hpp>
#include <iostream>

#include "../../../files/files.h"
#include "../../../physics/Hitbox.h"
#include "../../../objects/Player.h"
#include "../../../world/Level.h"
#include "../../../world/World.h"
#include "../../../content/Content.h"
#include "../../../voxels/Block.h"
#include "../../../voxels/Chunks.h"
#include "../../../voxels/voxel.h"
#include "../../../items/ItemDef.h"
#include "../../../items/ItemStack.h"
#include "../../../items/Inventory.h"
#include "../../../items/Inventories.h"
#include "../../../lighting/Lighting.h"
#include "../../../logic/BlocksController.h"
#include "../../../window/Window.h"
#include "../../../engine.h"

fs::path resolve_path(lua_State* L, const std::string& path) {
    try {
        return scripting::engine->getPaths()->resolve(path);
    } catch (const files_access_error& err) {
        luaL_error(L, err.what());
        abort(); // unreachable
    }
}

/* == file library == */
int l_file_resolve(lua_State* L) {
    fs::path path = resolve_path(L, lua_tostring(L, 1));
    lua_pushstring(L, path.u8string().c_str());
    return 1;
}

int l_file_read(lua_State* L) {
    fs::path path = resolve_path(L, lua_tostring(L, 1));
    if (fs::is_regular_file(path)) {
        lua_pushstring(L, files::read_string(path).c_str());
        return 1;
    }
    return luaL_error(L, "file does not exists '%s'", path.u8string().c_str());
}

int l_file_write(lua_State* L) {
    fs::path path = resolve_path(L, lua_tostring(L, 1));
    const char* text = lua_tostring(L, 2);
    files::write_string(path, text);
    return 1;    
}

int l_file_exists(lua_State* L) {
    fs::path path = resolve_path(L, lua_tostring(L, 1));
    lua_pushboolean(L, fs::exists(path));
    return 1;
}

int l_file_isfile(lua_State* L) {
    fs::path path = resolve_path(L, lua_tostring(L, 1));
    lua_pushboolean(L, fs::is_regular_file(path));
    return 1;
}

int l_file_isdir(lua_State* L) {
    fs::path path = resolve_path(L, lua_tostring(L, 1));
    lua_pushboolean(L, fs::is_directory(path));
    return 1;
}

int l_file_length(lua_State* L) {
    fs::path path = resolve_path(L, lua_tostring(L, 1));
    if (fs::exists(path)){
        lua_pushinteger(L, fs::file_size(path));
    } else {
        lua_pushinteger(L, -1);
    }
    return 1;
}

int l_file_mkdir(lua_State* L) {
    fs::path path = resolve_path(L, lua_tostring(L, 1));
    lua_pushboolean(L, fs::create_directory(path));
    return 1;    
}

/* == time library == */
int l_time_uptime(lua_State* L) {
    lua_pushnumber(L, Window::time());
    return 1;
}

int l_time_delta(lua_State* L) {
    lua_pushnumber(L, scripting::engine->getDelta());
    return 1;
}

/* == pack library == */
int l_pack_get_folder(lua_State* L) {
    std::string packName = lua_tostring(L, 1);
    if (packName == "core") {
        auto folder = scripting::engine->getPaths()
                                       ->getResources().u8string()+"/";
        lua_pushstring(L, folder.c_str());
        return 1;
    }
    for (auto& pack : scripting::engine->getContentPacks()) {
        if (pack.id == packName) {
            lua_pushstring(L, (pack.folder.u8string()+"/").c_str());
            return 1;
        }
    }
    lua_pushstring(L, "");
    return 1;
}

/* == world library == */
int l_world_get_total_time(lua_State* L) {
    lua_pushnumber(L, scripting::level->world->totalTime);
    return 1;
}

int l_world_get_day_time(lua_State* L) {
    lua_pushnumber(L, scripting::level->world->daytime);
    return 1;
}

int l_world_set_day_time(lua_State* L) {
    double value = lua_tonumber(L, 1);
    scripting::level->world->daytime = fmod(value, 1.0);
    return 0;
}

int l_world_get_seed(lua_State* L) {
    lua_pushinteger(L, scripting::level->world->getSeed());
    return 1;
}

/* == player library ==*/
int l_player_get_pos(lua_State* L) {
    int playerid = lua_tointeger(L, 1);
    if (playerid != 1)
        return 0;
    glm::vec3 pos = scripting::level->player->hitbox->position;
    lua_pushnumber(L, pos.x);
    lua_pushnumber(L, pos.y);
    lua_pushnumber(L, pos.z);
    return 3;
}

int l_player_get_rot(lua_State* L) {
    int playerid = lua_tointeger(L, 1);
    if (playerid != 1)
        return 0;
    glm::vec2 rot = scripting::level->player->cam;
    lua_pushnumber(L, rot.x);
    lua_pushnumber(L, rot.y);
    return 2;
}

int l_player_set_rot(lua_State* L) {
    int playerid = lua_tointeger(L, 1);
    if (playerid != 1)
        return 0;
    lua::luanumber x = lua_tonumber(L, 2);
    lua::luanumber y = lua_tonumber(L, 3);
    glm::vec2& cam = scripting::level->player->cam;
    cam.x = x;
    cam.y = y;
    return 0;
}

int l_player_set_pos(lua_State* L) {
    int playerid = lua_tointeger(L, 1);
    if (playerid != 1)
        return 0;
    lua::luanumber x = lua_tonumber(L, 2);
    lua::luanumber y = lua_tonumber(L, 3);
    lua::luanumber z = lua_tonumber(L, 4);
    scripting::level->player->hitbox->position = glm::vec3(x, y, z);
    return 0;
}

int l_player_get_inv(lua_State* L) {
    int playerid = lua_tointeger(L, 1);
    if (playerid != 1)
        return 0;
    Player* player = scripting::level->player;
    lua_pushinteger(L, player->getInventory()->getId());
    lua_pushinteger(L, player->getChosenSlot());
    return 2;
}

static void validate_itemid(lua_State* L, itemid_t id) {
    if (id >= scripting::indices->countItemDefs()) {
        luaL_error(L, "invalid item id");
    }
}

/* == inventory library == */
int l_inventory_get(lua_State* L) {
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

int l_inventory_set(lua_State* L) {
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

int l_inventory_size(lua_State* L) {
    lua::luaint invid = lua_tointeger(L, 1);
    auto inv = scripting::level->inventories->get(invid);
    if (inv == nullptr) {
        luaL_error(L, "inventory does not exists in runtime: %d", invid);
    }
    lua_pushinteger(L, inv->size());
    return 1;
}

int l_inventory_add(lua_State* L) {
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

int l_inventory_get_block(lua_State* L) {
    lua::luaint x = lua_tointeger(L, 1);
    lua::luaint y = lua_tointeger(L, 2);
    lua::luaint z = lua_tointeger(L, 3);
    int64_t id = scripting::blocks->createBlockInventory(x, y, z);
    lua_pushinteger(L, id);
    return 1;
}

/* == item library == */
int l_item_name(lua_State* L) {
    auto indices = scripting::content->getIndices();
    lua::luaint id = lua_tointeger(L, 1);
    if (id < 0 || size_t(id) >= indices->countItemDefs()) {
        return 0;
    }
    auto def = indices->getItemDef(id);
    lua_pushstring(L, def->name.c_str());
    return 1;
}

int l_item_index(lua_State* L) {
    auto name = lua_tostring(L, 1);
    lua_pushinteger(L, scripting::content->requireItem(name).rt.id);
    return 1;
}

int l_item_stack_size(lua_State* L) {
    auto indices = scripting::content->getIndices();
    lua::luaint id = lua_tointeger(L, 1);
    if (id < 0 || size_t(id) >= indices->countItemDefs()) {
        return 0;
    }
    auto def = indices->getItemDef(id);
    lua_pushinteger(L, def->stackSize);
    return 1;
}

int l_item_defs_count(lua_State* L) {
    lua_pushinteger(L, scripting::indices->countItemDefs());
    return 1;
}

/* == blocks-related functions == */
int l_block_name(lua_State* L) {
    auto indices = scripting::content->getIndices();
    lua::luaint id = lua_tointeger(L, 1);
    if (id < 0 || size_t(id) >= indices->countBlockDefs()) {
        return 0;
    }
    auto def = indices->getBlockDef(id);
    lua_pushstring(L, def->name.c_str());
    return 1;
}

int l_is_solid_at(lua_State* L) {
    lua::luaint x = lua_tointeger(L, 1);
    lua::luaint y = lua_tointeger(L, 2);
    lua::luaint z = lua_tointeger(L, 3);

    lua_pushboolean(L, scripting::level->chunks->isSolidBlock(x, y, z));
    return 1;
}

int l_blocks_count(lua_State* L) {
    lua_pushinteger(L, scripting::indices->countBlockDefs());
    return 1;
}

int l_block_index(lua_State* L) {
    auto name = lua_tostring(L, 1);
    lua_pushinteger(L, scripting::content->requireBlock(name).rt.id);
    return 1;
}

int l_set_block(lua_State* L) {
    lua::luaint x = lua_tointeger(L, 1);
    lua::luaint y = lua_tointeger(L, 2);
    lua::luaint z = lua_tointeger(L, 3);
    lua::luaint id = lua_tointeger(L, 4);    
    lua::luaint states = lua_tointeger(L, 5);
    bool noupdate = lua_toboolean(L, 6);
    if (id < 0 || size_t(id) >= scripting::indices->countBlockDefs()) {
        return 0;
    }
    scripting::level->chunks->set(x, y, z, id, states);
    scripting::level->lighting->onBlockSet(x,y,z, id);
    if (!noupdate)
        scripting::blocks->updateSides(x, y, z);
    return 0;
}

int l_get_block(lua_State* L) {
    lua::luaint x = lua_tointeger(L, 1);
    lua::luaint y = lua_tointeger(L, 2);
    lua::luaint z = lua_tointeger(L, 3);
    voxel* vox = scripting::level->chunks->get(x, y, z);
    int id = vox == nullptr ? -1 : vox->id;
    lua_pushinteger(L, id);
    return 1;
}

int l_get_block_x(lua_State* L) {
    lua::luaint x = lua_tointeger(L, 1);
    lua::luaint y = lua_tointeger(L, 2);
    lua::luaint z = lua_tointeger(L, 3);
    voxel* vox = scripting::level->chunks->get(x, y, z);
    if (vox == nullptr) {
        return lua::pushivec3(L, 1, 0, 0);
    }
    auto def = scripting::level->content->getIndices()->getBlockDef(vox->id);
    if (!def->rotatable) {
        return lua::pushivec3(L, 1, 0, 0);
    } else {
        const CoordSystem& rot = def->rotations.variants[vox->rotation()];
        return lua::pushivec3(L, rot.axisX.x, rot.axisX.y, rot.axisX.z);
    }
}

int l_get_block_y(lua_State* L) {
    lua::luaint x = lua_tointeger(L, 1);
    lua::luaint y = lua_tointeger(L, 2);
    lua::luaint z = lua_tointeger(L, 3);
    voxel* vox = scripting::level->chunks->get(x, y, z);
    if (vox == nullptr) {
        return lua::pushivec3(L, 0, 1, 0);
    }
    auto def = scripting::level->content->getIndices()->getBlockDef(vox->id);
    if (!def->rotatable) {
        return lua::pushivec3(L, 0, 1, 0);
    } else {
        const CoordSystem& rot = def->rotations.variants[vox->rotation()];
        return lua::pushivec3(L, rot.axisY.x, rot.axisY.y, rot.axisY.z);
    }
}

int l_get_block_z(lua_State* L) {
    lua::luaint x = lua_tointeger(L, 1);
    lua::luaint y = lua_tointeger(L, 2);
    lua::luaint z = lua_tointeger(L, 3);
    voxel* vox = scripting::level->chunks->get(x, y, z);
    if (vox == nullptr) {
        return lua::pushivec3(L, 0, 0, 1);
    }
    auto def = scripting::level->content->getIndices()->getBlockDef(vox->id);
    if (!def->rotatable) {
        return lua::pushivec3(L, 0, 0, 1);
    } else {
        const CoordSystem& rot = def->rotations.variants[vox->rotation()];
        return lua::pushivec3(L, rot.axisZ.x, rot.axisZ.y, rot.axisZ.z);
    }
}

int l_get_block_states(lua_State* L) {
    lua::luaint x = lua_tointeger(L, 1);
    lua::luaint y = lua_tointeger(L, 2);
    lua::luaint z = lua_tointeger(L, 3);
    voxel* vox = scripting::level->chunks->get(x, y, z);
    int states = vox == nullptr ? 0 : vox->states;
    lua_pushinteger(L, states);
    return 1;
}

int l_get_block_user_bits(lua_State* L) {
    lua::luaint x = lua_tointeger(L, 1);
    lua::luaint y = lua_tointeger(L, 2);
    lua::luaint z = lua_tointeger(L, 3);
    lua::luaint offset = lua_tointeger(L, 4) + VOXEL_USER_BITS_OFFSET;
    lua::luaint bits = lua_tointeger(L, 5);

    voxel* vox = scripting::level->chunks->get(x, y, z);
    if (vox == nullptr) {
        lua_pushinteger(L, 0);
        return 1;
    }
    uint mask = ((1 << bits) - 1) << offset;
    uint data = (vox->states & mask) >> offset;
    lua_pushinteger(L, data);
    return 1;
}

int l_set_block_user_bits(lua_State* L) {
    lua::luaint x = lua_tointeger(L, 1);
    lua::luaint y = lua_tointeger(L, 2);
    lua::luaint z = lua_tointeger(L, 3);
    lua::luaint offset = lua_tointeger(L, 4) + VOXEL_USER_BITS_OFFSET;
    lua::luaint bits = lua_tointeger(L, 5);

    uint mask = ((1 << bits) - 1) << offset;
    lua::luaint value = (lua_tointeger(L, 6) << offset) & mask;
    
    voxel* vox = scripting::level->chunks->get(x, y, z);
    if (vox == nullptr) {
        return 0;
    }
    vox->states = (vox->states & (~mask)) | value;
    return 0;
}

int l_is_replaceable_at(lua_State* L) {
    int x = lua_tointeger(L, 1);
    int y = lua_tointeger(L, 2);
    int z = lua_tointeger(L, 3);

    lua_pushboolean(L, scripting::level->chunks->isReplaceableBlock(x, y, z));
    return 1;
}

int l_print(lua_State* L) {
    int n = lua_gettop(L);  /* number of arguments */
    lua_getglobal(L, "tostring");
    for (int i=1; i<=n; i++) {
        lua_pushvalue(L, -1);  /* function to be called */
        lua_pushvalue(L, i);   /* value to print */
        lua_call(L, 1, 1);
        const char* s = lua_tostring(L, -1);  /* get result */
        if (s == NULL)
            return luaL_error(L, LUA_QL("tostring") " must return a string to "
                                 LUA_QL("print"));
        if (i > 1)
            std::cout << "\t";
        std::cout << s;
        lua_pop(L, 1);  /* pop result */
    }
    std::cout << std::endl;
    return 0;
}
