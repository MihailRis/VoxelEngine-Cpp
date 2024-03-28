#include "lua_commons.h"
#include "api_lua.h"
#include "lua_util.h"
#include "../scripting.h"
#include "../../../world/Level.h"
#include "../../../voxels/ChunksStorage.h"
#include "../../../voxels/Chunk.h"
#include "../../../voxels/Block.h"
#include "../../../voxels/voxel.h"
#include "../../../lighting/Lighting.h"
#include "../../../content/Content.h"
#include "../../../logic/BlocksController.h"

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

    lua_pushboolean(L, scripting::level->chunksStorage->isSolidBlock(x, y, z));
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
    scripting::level->chunksStorage->setVoxel(x, y, z, id, states);
    scripting::level->lighting->onBlockSet(x,y,z, id);
    if (!noupdate)
        scripting::blocks->updateSides(x, y, z);
    return 0;
}

int l_get_block(lua_State* L) {
    lua::luaint x = lua_tointeger(L, 1);
    lua::luaint y = lua_tointeger(L, 2);
    lua::luaint z = lua_tointeger(L, 3);
    voxel* vox = scripting::level->chunksStorage->getVoxel(x, y, z);
    int id = vox == nullptr ? -1 : vox->id;
    lua_pushinteger(L, id);
    return 1;
}

int l_get_block_x(lua_State* L) {
    lua::luaint x = lua_tointeger(L, 1);
    lua::luaint y = lua_tointeger(L, 2);
    lua::luaint z = lua_tointeger(L, 3);
    voxel* vox = scripting::level->chunksStorage->getVoxel(x, y, z);
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
    voxel* vox = scripting::level->chunksStorage->getVoxel(x, y, z);
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
    voxel* vox = scripting::level->chunksStorage->getVoxel(x, y, z);
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

int l_get_block_rotation(lua_State* L) {
    lua::luaint x = lua_tointeger(L, 1);
    lua::luaint y = lua_tointeger(L, 2);
    lua::luaint z = lua_tointeger(L, 3);
    voxel* vox = scripting::level->chunksStorage->getVoxel(x, y, z);
    int rotation = vox == nullptr ? 0 : vox->rotation();
    lua_pushinteger(L, rotation);
    return 1;
}

int l_set_block_rotation(lua_State* L) {
    lua::luaint x = lua_tointeger(L, 1);
    lua::luaint y = lua_tointeger(L, 2);
    lua::luaint z = lua_tointeger(L, 3);
    lua::luaint value = lua_tointeger(L, 4);
    voxel* vox = scripting::level->chunksStorage->getVoxel(x, y, z);
    if (vox == nullptr) {
        return 0;
    }
    vox->setRotation(value);
    scripting::level->chunksStorage->getChunkByVoxel(x, y, z)->setModified(true);
    return 0;
}

int l_get_block_states(lua_State* L) {
    lua::luaint x = lua_tointeger(L, 1);
    lua::luaint y = lua_tointeger(L, 2);
    lua::luaint z = lua_tointeger(L, 3);
    voxel* vox = scripting::level->chunksStorage->getVoxel(x, y, z);
    int states = vox == nullptr ? 0 : vox->states;
    lua_pushinteger(L, states);
    return 1;
}

int l_set_block_states(lua_State* L) {
    lua::luaint x = lua_tointeger(L, 1);
    lua::luaint y = lua_tointeger(L, 2);
    lua::luaint z = lua_tointeger(L, 3);
    lua::luaint states = lua_tointeger(L, 4);

    Chunk* chunk = scripting::level->chunksStorage->getChunkByVoxel(x, y, z);
    if (chunk == nullptr) {
        return 0;
    }
    voxel* vox = scripting::level->chunksStorage->getVoxel(x, y, z);
    vox->states = states;
    chunk->setModified(true);
    return 0;
}

int l_get_block_user_bits(lua_State* L) {
    lua::luaint x = lua_tointeger(L, 1);
    lua::luaint y = lua_tointeger(L, 2);
    lua::luaint z = lua_tointeger(L, 3);
    lua::luaint offset = lua_tointeger(L, 4) + VOXEL_USER_BITS_OFFSET;
    lua::luaint bits = lua_tointeger(L, 5);

    voxel* vox = scripting::level->chunksStorage->getVoxel(x, y, z);
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
    
    voxel* vox = scripting::level->chunksStorage->getVoxel(x, y, z);
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

    lua_pushboolean(L, scripting::level->chunksStorage->isReplaceableBlock(x, y, z));
    return 1;
}

const luaL_Reg blocklib [] = {
    {"index", lua_wrap_errors<l_block_index>},
    {"name", lua_wrap_errors<l_block_name>},
    {"defs_count", lua_wrap_errors<l_blocks_count>},
    {"is_solid_at", lua_wrap_errors<l_is_solid_at>},
    {"is_replaceable_at", lua_wrap_errors<l_is_replaceable_at>},
    {"set", lua_wrap_errors<l_set_block>},
    {"get", lua_wrap_errors<l_get_block>},
    {"get_X", lua_wrap_errors<l_get_block_x>},
    {"get_Y", lua_wrap_errors<l_get_block_y>},
    {"get_Z", lua_wrap_errors<l_get_block_z>},
    {"get_states", lua_wrap_errors<l_get_block_states>},
    {"set_states", lua_wrap_errors<l_set_block_states>},
    {"get_rotation", lua_wrap_errors<l_get_block_rotation>},
    {"set_rotation", lua_wrap_errors<l_set_block_rotation>},
    {"get_user_bits", lua_wrap_errors<l_get_block_user_bits>},
    {"set_user_bits", lua_wrap_errors<l_set_block_user_bits>},
    {NULL, NULL}
};
