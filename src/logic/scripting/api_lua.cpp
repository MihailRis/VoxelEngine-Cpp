#include "api_lua.h"
#include "scripting.h"

#include <glm/glm.hpp>

#include "../../files/files.h"
#include "../../physics/Hitbox.h"
#include "../../objects/Player.h"
#include "../../world/Level.h"
#include "../../world/World.h"
#include "../../content/Content.h"
#include "../../voxels/Block.h"
#include "../../voxels/Chunks.h"
#include "../../voxels/voxel.h"
#include "../../lighting/Lighting.h"
#include "../../logic/BlocksController.h"
#include "../../window/Window.h"
#include "../../engine.h"

inline int lua_pushivec3(lua_State* L, int x, int y, int z) {
    lua_pushinteger(L, x);
    lua_pushinteger(L, y);
    lua_pushinteger(L, z);
    return 3;
}

inline void luaL_openlib(lua_State* L, const char* name, const luaL_Reg* libfuncs, int nup) {
    lua_newtable(L);
    luaL_setfuncs(L, libfuncs, nup);
    lua_setglobal(L, name);
}

/* == file library == */
static int l_file_resolve(lua_State* L) {
    std::string path = lua_tostring(L, 1);
    fs::path resolved = scripting::engine->getPaths()->resolve(path);
    lua_pushstring(L, resolved.u8string().c_str());
    return 1;
}

static int l_file_read(lua_State* L) {
    auto paths = scripting::engine->getPaths();
    fs::path path = paths->resolve(lua_tostring(L, 1));
    if (fs::is_regular_file(path)) {
        lua_pushstring(L, files::read_string(path).c_str());
        return 1;
    }
    return luaL_error(L, "file does not exists '%s'", path.u8string().c_str());
}

static int l_file_write(lua_State* L) {
    auto paths = scripting::engine->getPaths();
    fs::path path = paths->resolve(lua_tostring(L, 1));
    const char* text = lua_tostring(L, 2);
    files::write_string(path, text);
    return 1;    
}

static int l_file_exists(lua_State* L) {
    auto paths = scripting::engine->getPaths();
    fs::path path = paths->resolve(lua_tostring(L, 1));
    lua_pushboolean(L, fs::exists(path));
    return 1;
}

static int l_file_isfile(lua_State* L) {
    auto paths = scripting::engine->getPaths();
    fs::path path = paths->resolve(lua_tostring(L, 1));
    lua_pushboolean(L, fs::is_regular_file(path));
    return 1;
}

static int l_file_isdir(lua_State* L) {
    auto paths = scripting::engine->getPaths();
    fs::path path = paths->resolve(lua_tostring(L, 1));
    lua_pushboolean(L, fs::is_directory(path));
    return 1;
}

static int l_file_length(lua_State* L) {
    auto paths = scripting::engine->getPaths();
    fs::path path = paths->resolve(lua_tostring(L, 1));
    if (fs::exists(path)){
        lua_pushinteger(L, fs::file_size(path));
    } else {
        lua_pushinteger(L, -1);
    }
    return 1;
}

static int l_file_mkdir(lua_State* L) {
    auto paths = scripting::engine->getPaths();
    fs::path path = paths->resolve(lua_tostring(L, 1));
    lua_pushboolean(L, fs::create_directory(path));
    return 1;    
}

static const luaL_Reg filelib [] = {
    {"resolve", l_file_resolve},
    {"read", l_file_read},
    {"write", l_file_write},
    {"exists", l_file_exists},
    {"isfile", l_file_isfile},
    {"isdir", l_file_isdir},
    {"length", l_file_length},
    {"mkdir", l_file_mkdir},
    {NULL, NULL}
};

/* == time library == */
static int l_time_uptime(lua_State* L) {
    lua_pushnumber(L, Window::time());
    return 1;
}

static const luaL_Reg timelib [] = {
    {"uptime", l_time_uptime},
    {NULL, NULL}
};
 
/* == pack library == */
static int l_pack_get_folder(lua_State* L) {
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

static const luaL_Reg packlib [] = {
    {"get_folder", l_pack_get_folder},
    {NULL, NULL}
};

/* == world library == */
static int l_world_get_total_time(lua_State* L) {
    lua_pushnumber(L, scripting::level->world->totalTime);
    return 1;
}

static int l_world_get_day_time(lua_State* L) {
    lua_pushnumber(L, scripting::level->world->daytime);
    return 1;
}

static int l_world_set_day_time(lua_State* L) {
    double value = lua_tonumber(L, 1);
    scripting::level->world->daytime = fmod(value, 1.0);
    return 0;
}

static int l_world_get_seed(lua_State* L) {
    lua_pushinteger(L, scripting::level->world->getSeed());
    return 1;
}

static const luaL_Reg worldlib [] = {
    {"get_total_time", l_world_get_total_time},
    {"get_day_time", l_world_get_day_time},
    {"set_day_time", l_world_set_day_time},
    {"get_seed", l_world_get_seed},
    {NULL, NULL}
};

/* == player library ==*/
static int l_player_get_pos(lua_State* L) {
    int playerid = lua_tointeger(L, 1);
    if (playerid != 1)
        return 0;
    glm::vec3 pos = scripting::level->player->hitbox->position;
    lua_pushnumber(L, pos.x);
    lua_pushnumber(L, pos.y);
    lua_pushnumber(L, pos.z);
    return 3;
}

static int l_player_get_rot(lua_State* L) {
    int playerid = lua_tointeger(L, 1);
    if (playerid != 1)
        return 0;
    glm::vec2 rot = scripting::level->player->cam;
    lua_pushnumber(L, rot.x);
    lua_pushnumber(L, rot.y);
    return 2;
}

static int l_player_set_rot(lua_State* L) {
    int playerid = lua_tointeger(L, 1);
    if (playerid != 1)
        return 0;
    double x = lua_tonumber(L, 2);
    double y = lua_tonumber(L, 3);
    glm::vec2& cam = scripting::level->player->cam;
    cam.x = x;
    cam.y = y;
    return 0;
}

static int l_player_set_pos(lua_State* L) {
    int playerid = lua_tointeger(L, 1);
    if (playerid != 1)
        return 0;
    double x = lua_tonumber(L, 2);
    double y = lua_tonumber(L, 3);
    double z = lua_tonumber(L, 4);
    scripting::level->player->hitbox->position = glm::vec3(x, y, z);
    return 0;
}

static const luaL_Reg playerlib [] = {
    {"get_pos", l_player_get_pos},
    {"set_pos", l_player_set_pos},
    {"get_rot", l_player_get_rot},
    {"set_rot", l_player_set_rot},
    {NULL, NULL}
};

/* == blocks-related functions == */
static int l_block_name(lua_State* L) {
    int id = lua_tointeger(L, 1);
    auto def = scripting::content->getIndices()->getBlockDef(id);
    lua_pushstring(L, def->name.c_str());
    return 1;
}

static int l_is_solid_at(lua_State* L) {
    int x = lua_tointeger(L, 1);
    int y = lua_tointeger(L, 2);
    int z = lua_tointeger(L, 3);

    lua_pushboolean(L, scripting::level->chunks->isSolidBlock(x, y, z));
    return 1;
}

static int l_blocks_count(lua_State* L) {
    lua_pushinteger(L, scripting::content->getIndices()->countBlockDefs());
    return 1;
}

static int l_block_index(lua_State* L) {
    auto name = lua_tostring(L, 1);
    lua_pushinteger(L, scripting::content->requireBlock(name)->rt.id);
    return 1;
}

static int l_set_block(lua_State* L) {
    int x = lua_tointeger(L, 1);
    int y = lua_tointeger(L, 2);
    int z = lua_tointeger(L, 3);
    int id = lua_tointeger(L, 4);    
    int states = lua_tointeger(L, 5);
    bool noupdate = lua_toboolean(L, 6);
    scripting::level->chunks->set(x, y, z, id, states);
    scripting::level->lighting->onBlockSet(x,y,z, id);
    if (!noupdate)
        scripting::blocks->updateSides(x, y, z);
    return 0;
}

static int l_get_block(lua_State* L) {
    int x = lua_tointeger(L, 1);
    int y = lua_tointeger(L, 2);
    int z = lua_tointeger(L, 3);
    voxel* vox = scripting::level->chunks->get(x, y, z);
    int id = vox == nullptr ? -1 : vox->id;
    lua_pushinteger(L, id);
    return 1;
}

static int l_get_block_x(lua_State* L) {
    int x = lua_tointeger(L, 1);
    int y = lua_tointeger(L, 2);
    int z = lua_tointeger(L, 3);
    voxel* vox = scripting::level->chunks->get(x, y, z);
    if (vox == nullptr) {
        return lua_pushivec3(L, 1, 0, 0);
    }
    auto def = scripting::level->content->getIndices()->getBlockDef(vox->id);
    if (!def->rotatable) {
        return lua_pushivec3(L, 1, 0, 0);
    } else {
        const CoordSystem& rot = def->rotations.variants[vox->rotation()];
        return lua_pushivec3(L, rot.axisX.x, rot.axisX.y, rot.axisX.z);
    }
}

static int l_get_block_y(lua_State* L) {
    int x = lua_tointeger(L, 1);
    int y = lua_tointeger(L, 2);
    int z = lua_tointeger(L, 3);
    voxel* vox = scripting::level->chunks->get(x, y, z);
    if (vox == nullptr) {
        return lua_pushivec3(L, 0, 1, 0);
    }
    auto def = scripting::level->content->getIndices()->getBlockDef(vox->id);
    if (!def->rotatable) {
        return lua_pushivec3(L, 0, 1, 0);
    } else {
        const CoordSystem& rot = def->rotations.variants[vox->rotation()];
        return lua_pushivec3(L, rot.axisY.x, rot.axisY.y, rot.axisY.z);
    }
}

static int l_get_block_z(lua_State* L) {
    int x = lua_tointeger(L, 1);
    int y = lua_tointeger(L, 2);
    int z = lua_tointeger(L, 3);
    voxel* vox = scripting::level->chunks->get(x, y, z);
    if (vox == nullptr) {
        return lua_pushivec3(L, 0, 0, 1);
    }
    auto def = scripting::level->content->getIndices()->getBlockDef(vox->id);
    if (!def->rotatable) {
        return lua_pushivec3(L, 0, 0, 1);
    } else {
        const CoordSystem& rot = def->rotations.variants[vox->rotation()];
        return lua_pushivec3(L, rot.axisZ.x, rot.axisZ.y, rot.axisZ.z);
    }
}

static int l_get_block_states(lua_State* L) {
    int x = lua_tointeger(L, 1);
    int y = lua_tointeger(L, 2);
    int z = lua_tointeger(L, 3);
    voxel* vox = scripting::level->chunks->get(x, y, z);
    int states = vox == nullptr ? 0 : vox->states;
    lua_pushinteger(L, states);
    return 1;
}

static int l_get_block_user_bits(lua_State* L) {
    int x = lua_tointeger(L, 1);
    int y = lua_tointeger(L, 2);
    int z = lua_tointeger(L, 3);
    int offset = lua_tointeger(L, 4) + VOXEL_USER_BITS_OFFSET;
    int bits = lua_tointeger(L, 5);

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

static int l_set_block_user_bits(lua_State* L) {
    int x = lua_tointeger(L, 1);
    int y = lua_tointeger(L, 2);
    int z = lua_tointeger(L, 3);
    int offset = lua_tointeger(L, 4) + VOXEL_USER_BITS_OFFSET;
    int bits = lua_tointeger(L, 5);

    uint mask = ((1 << bits) - 1) << offset;
    int value = (lua_tointeger(L, 6) << offset) & mask;
    
    voxel* vox = scripting::level->chunks->get(x, y, z);
    if (vox == nullptr) {
        return 0;
    }
    vox->states = (vox->states & (~mask)) | value;
    return 0;
}

static int l_is_replaceable_at(lua_State* L) {
    int x = lua_tointeger(L, 1);
    int y = lua_tointeger(L, 2);
    int z = lua_tointeger(L, 3);

    lua_pushboolean(L, scripting::level->chunks->isReplaceableBlock(x, y, z));
    return 1;
}

#define lua_addfunc(L, FUNC, NAME) (lua_pushcfunction(L, FUNC),\
                                    lua_setglobal(L, NAME))

void apilua::create_funcs(lua_State* L) {
    luaL_openlib(L, "pack", packlib, 0);
    luaL_openlib(L, "world", worldlib, 0);
    luaL_openlib(L, "player", playerlib, 0);
    luaL_openlib(L, "time", timelib, 0);
    luaL_openlib(L, "file", filelib, 0);

    lua_addfunc(L, l_block_index, "block_index");
    lua_addfunc(L, l_block_name, "block_name");
    lua_addfunc(L, l_blocks_count, "blocks_count");
    lua_addfunc(L, l_is_solid_at, "is_solid_at");
    lua_addfunc(L, l_is_replaceable_at, "is_replaceable_at");
    lua_addfunc(L, l_set_block, "set_block");
    lua_addfunc(L, l_get_block, "get_block");
    lua_addfunc(L, l_get_block_x, "get_block_X");
    lua_addfunc(L, l_get_block_y, "get_block_Y");
    lua_addfunc(L, l_get_block_z, "get_block_Z");
    lua_addfunc(L, l_get_block_states, "get_block_states");
    lua_addfunc(L, l_get_block_user_bits, "get_block_user_bits");
    lua_addfunc(L, l_set_block_user_bits, "set_block_user_bits");
}
