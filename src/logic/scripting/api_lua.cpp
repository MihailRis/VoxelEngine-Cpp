#include "api_lua.h"
#include "scripting.h"

#include <glm/glm.hpp>

#include "../../physics/Hitbox.h"
#include "../../objects/Player.h"
#include "../../world/Level.h"
#include "../../content/Content.h"
#include "../../voxels/Block.h"
#include "../../voxels/Chunks.h"
#include "../../voxels/voxel.h"
#include "../../lighting/Lighting.h"

int l_block_name(lua_State* L) {
    int id = lua_tointeger(L, 1);
    lua_pushstring(L, scripting::content->indices->getBlockDef(id)->name.c_str());
    return 1;
}

int l_is_solid_at(lua_State* L) {
    int x = lua_tointeger(L, 1);
    int y = lua_tointeger(L, 2);
    int z = lua_tointeger(L, 3);

    lua_pushboolean(L, scripting::level->chunks->isSolid(x, y, z));
    return 1;
}

int l_blocks_count(lua_State* L) {
    lua_pushinteger(L, scripting::content->indices->countBlockDefs());
    return 1;
}

int l_block_index(lua_State* L) {
    auto name = lua_tostring(L, 1);
    lua_pushinteger(L, scripting::content->requireBlock(name)->rt.id);
    return 1;
}

int l_set_block(lua_State* L) {
    int x = lua_tointeger(L, 1);
    int y = lua_tointeger(L, 2);
    int z = lua_tointeger(L, 3);
    int id = lua_tointeger(L, 4);    
    int states = lua_tointeger(L, 5);
    scripting::level->chunks->set(x, y, z, id, states);
    scripting::level->lighting->onBlockSet(x,y,z, id);
    return 0;
}

int l_get_block(lua_State* L) {
    int x = lua_tointeger(L, 1);
    int y = lua_tointeger(L, 2);
    int z = lua_tointeger(L, 3);
    voxel* vox = scripting::level->chunks->get(x, y, z);
    int id = vox == nullptr ? -1 : vox->id;
    lua_pushinteger(L, id);
    return 1;
}

inline int lua_pushivec3(lua_State* L, int x, int y, int z) {
    lua_pushinteger(L, x);
    lua_pushinteger(L, y);
    lua_pushinteger(L, z);
    return 3;
}

int l_get_block_x(lua_State* L) {
    int x = lua_tointeger(L, 1);
    int y = lua_tointeger(L, 2);
    int z = lua_tointeger(L, 3);
    voxel* vox = scripting::level->chunks->get(x, y, z);
    if (vox == nullptr) {
        return lua_pushivec3(L, 1, 0, 0);
    }
    const Block* def = scripting::level->content->indices->getBlockDef(vox->id);
    if (!def->rotatable) {
        return lua_pushivec3(L, 1, 0, 0);
    } else {
        const CoordSystem& rot = def->rotations.variants[vox->rotation()];
        return lua_pushivec3(L, rot.axisX.x, rot.axisX.y, rot.axisX.z);
    }
}

int l_get_block_y(lua_State* L) {
    int x = lua_tointeger(L, 1);
    int y = lua_tointeger(L, 2);
    int z = lua_tointeger(L, 3);
    voxel* vox = scripting::level->chunks->get(x, y, z);
    if (vox == nullptr) {
        return lua_pushivec3(L, 0, 1, 0);
    }
    const Block* def = scripting::level->content->indices->getBlockDef(vox->id);
    if (!def->rotatable) {
        return lua_pushivec3(L, 0, 1, 0);
    } else {
        const CoordSystem& rot = def->rotations.variants[vox->rotation()];
        return lua_pushivec3(L, rot.axisY.x, rot.axisY.y, rot.axisY.z);
    }
}

int l_get_block_z(lua_State* L) {
    int x = lua_tointeger(L, 1);
    int y = lua_tointeger(L, 2);
    int z = lua_tointeger(L, 3);
    voxel* vox = scripting::level->chunks->get(x, y, z);
    if (vox == nullptr) {
        return lua_pushivec3(L, 0, 0, 1);
    }
    const Block* def = scripting::level->content->indices->getBlockDef(vox->id);
    if (!def->rotatable) {
        return lua_pushivec3(L, 0, 0, 1);
    } else {
        const CoordSystem& rot = def->rotations.variants[vox->rotation()];
        return lua_pushivec3(L, rot.axisZ.x, rot.axisZ.y, rot.axisZ.z);
    }
}

int l_get_player_pos(lua_State* L) {
    glm::vec3 pos = scripting::level->player->hitbox->position;
    lua_pushnumber(L, pos.x);
    lua_pushnumber(L, pos.y);
    lua_pushnumber(L, pos.z);
    return 3;
}

int l_get_player_rot(lua_State* L) {
    glm::vec2 rot = scripting::level->player->cam;
    lua_pushnumber(L, rot.x);
    lua_pushnumber(L, rot.y);
    return 2;
}

int l_set_player_rot(lua_State* L) {
    double x = lua_tonumber(L, 1);
    double y = lua_tonumber(L, 2);
    glm::vec2& cam = scripting::level->player->cam;
    cam.x = x;
    cam.y = y;
    return 0;
}

int l_set_player_pos(lua_State* L) {
    double x = lua_tonumber(L, 1);
    double y = lua_tonumber(L, 2);
    double z = lua_tonumber(L, 3);
    scripting::level->player->hitbox->position = glm::vec3(x, y, z);
    return 0;
}

int l_get_block_states(lua_State* L) {
    int x = lua_tointeger(L, 1);
    int y = lua_tointeger(L, 2);
    int z = lua_tointeger(L, 3);
    voxel* vox = scripting::level->chunks->get(x, y, z);
    int states = vox == nullptr ? 0 : vox->states;
    lua_pushinteger(L, states);
    return 1;
}

int l_get_block_user_bits(lua_State* L) {
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

int l_set_block_user_bits(lua_State* L) {
    int x = lua_tointeger(L, 1);
    int y = lua_tointeger(L, 2);
    int z = lua_tointeger(L, 3);
    int offset = lua_tointeger(L, 4) + VOXEL_USER_BITS_OFFSET;
    int bits = lua_tointeger(L, 5);

    uint mask = (1 << bits) - 1;
    int value = lua_tointeger(L, 6) & mask;
    
    voxel* vox = scripting::level->chunks->get(x, y, z);
    if (vox == nullptr) {
        return 0;
    }
    vox->states = (vox->states & (~mask)) | (value << offset);
    return 0;
}

int l_is_replaceable_at(lua_State* L) {
    int x = lua_tointeger(L, 1);
    int y = lua_tointeger(L, 2);
    int z = lua_tointeger(L, 3);

    lua_pushboolean(L, scripting::level->chunks->isReplaceable(x, y, z));
    return 1;
}

#define lua_addfunc(L, FUNC, NAME) (lua_pushcfunction(L, FUNC),\
                                    lua_setglobal(L, NAME))

void apilua::create_funcs(lua_State* L) {
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
    lua_addfunc(L, l_get_player_pos, "get_player_pos");
    lua_addfunc(L, l_set_player_pos, "set_player_pos");
    lua_addfunc(L, l_get_player_rot, "get_player_rot");
    lua_addfunc(L, l_set_player_rot, "set_player_rot");
    lua_addfunc(L, l_get_block_states, "get_block_states");
    lua_addfunc(L, l_get_block_user_bits, "get_block_user_bits");
    lua_addfunc(L, l_set_block_user_bits, "set_block_user_bits");
}
