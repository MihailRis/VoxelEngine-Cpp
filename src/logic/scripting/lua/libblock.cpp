#include "lua_commons.hpp"

#include "api_lua.hpp"
#include "lua_util.hpp"
#include "../scripting.hpp"

#include "../../../world/Level.hpp"
#include "../../../voxels/Chunks.hpp"
#include "../../../voxels/Chunk.hpp"
#include "../../../voxels/Block.hpp"
#include "../../../voxels/voxel.hpp"
#include "../../../lighting/Lighting.hpp"
#include "../../../content/Content.hpp"
#include "../../../logic/BlocksController.hpp"

int l_block_name(lua_State* L) {
    auto indices = scripting::content->getIndices();
    lua_Integer id = lua_tointeger(L, 1);
    if (static_cast<size_t>(id) >= indices->countBlockDefs()) {
        return 0;
    }
    auto def = indices->getBlockDef(id);
    lua_pushstring(L, def->name.c_str());
    return 1;
}

int l_block_material(lua_State* L) {
    auto indices = scripting::content->getIndices();
    lua_Integer id = lua_tointeger(L, 1);
    if (static_cast<size_t>(id) >= indices->countBlockDefs()) {
        return 0;
    }
    auto def = indices->getBlockDef(id);
    lua_pushstring(L, def->material.c_str());
    return 1;
}

int l_is_solid_at(lua_State* L) {
    lua_Integer x = lua_tointeger(L, 1);
    lua_Integer y = lua_tointeger(L, 2);
    lua_Integer z = lua_tointeger(L, 3);

    lua_pushboolean(L, scripting::level->chunks->isSolidBlock(x, y, z));
    return 1;
}

int l_blocks_count(lua_State* L) {
    lua_pushinteger(L, scripting::indices->countBlockDefs());
    return 1;
}

int l_block_index(lua_State* L) {
    std::string name = lua_tostring(L, 1);
    lua_pushinteger(L, scripting::content->requireBlock(name).rt.id);
    return 1;
}

int l_set_block(lua_State* L) {
    lua_Integer x = lua_tointeger(L, 1);
    lua_Integer y = lua_tointeger(L, 2);
    lua_Integer z = lua_tointeger(L, 3);
    lua_Integer id = lua_tointeger(L, 4);    
    lua_Integer state = lua_tointeger(L, 5);
    bool noupdate = lua_toboolean(L, 6);
    if (static_cast<size_t>(id) >= scripting::indices->countBlockDefs()) {
        return 0;
    }
    if (!scripting::level->chunks->get(x, y, z)) {
        return 0;
    }
    scripting::level->chunks->set(x, y, z, id, int2blockstate(state));
    scripting::level->lighting->onBlockSet(x,y,z, id);
    if (!noupdate) {
        scripting::blocks->updateSides(x, y, z);
    }
    return 0;
}

int l_get_block(lua_State* L) {
    lua_Integer x = lua_tointeger(L, 1);
    lua_Integer y = lua_tointeger(L, 2);
    lua_Integer z = lua_tointeger(L, 3);
    voxel* vox = scripting::level->chunks->get(x, y, z);
    int id = vox == nullptr ? -1 : vox->id;
    lua_pushinteger(L, id);
    return 1;
}

int l_get_block_x(lua_State* L) {
    lua_Integer x = lua_tointeger(L, 1);
    lua_Integer y = lua_tointeger(L, 2);
    lua_Integer z = lua_tointeger(L, 3);
    voxel* vox = scripting::level->chunks->get(x, y, z);
    if (vox == nullptr) {
        return lua::pushivec3(L, 1, 0, 0);
    }
    auto def = scripting::level->content->getIndices()->getBlockDef(vox->id);
    if (!def->rotatable) {
        return lua::pushivec3(L, 1, 0, 0);
    } else {
        const CoordSystem& rot = def->rotations.variants[vox->state.rotation];
        return lua::pushivec3(L, rot.axisX.x, rot.axisX.y, rot.axisX.z);
    }
}

int l_get_block_y(lua_State* L) {
    lua_Integer x = lua_tointeger(L, 1);
    lua_Integer y = lua_tointeger(L, 2);
    lua_Integer z = lua_tointeger(L, 3);
    voxel* vox = scripting::level->chunks->get(x, y, z);
    if (vox == nullptr) {
        return lua::pushivec3(L, 0, 1, 0);
    }
    auto def = scripting::level->content->getIndices()->getBlockDef(vox->id);
    if (!def->rotatable) {
        return lua::pushivec3(L, 0, 1, 0);
    } else {
        const CoordSystem& rot = def->rotations.variants[vox->state.rotation];
        return lua::pushivec3(L, rot.axisY.x, rot.axisY.y, rot.axisY.z);
    }
}

int l_get_block_z(lua_State* L) {
    lua_Integer x = lua_tointeger(L, 1);
    lua_Integer y = lua_tointeger(L, 2);
    lua_Integer z = lua_tointeger(L, 3);
    voxel* vox = scripting::level->chunks->get(x, y, z);
    if (vox == nullptr) {
        return lua::pushivec3(L, 0, 0, 1);
    }
    auto def = scripting::level->content->getIndices()->getBlockDef(vox->id);
    if (!def->rotatable) {
        return lua::pushivec3(L, 0, 0, 1);
    } else {
        const CoordSystem& rot = def->rotations.variants[vox->state.rotation];
        return lua::pushivec3(L, rot.axisZ.x, rot.axisZ.y, rot.axisZ.z);
    }
}

int l_get_block_rotation(lua_State* L) {
    lua_Integer x = lua_tointeger(L, 1);
    lua_Integer y = lua_tointeger(L, 2);
    lua_Integer z = lua_tointeger(L, 3);
    voxel* vox = scripting::level->chunks->get(x, y, z);
    int rotation = vox == nullptr ? 0 : vox->state.rotation;
    lua_pushinteger(L, rotation);
    return 1;
}

int l_set_block_rotation(lua_State* L) {
    lua_Integer x = lua_tointeger(L, 1);
    lua_Integer y = lua_tointeger(L, 2);
    lua_Integer z = lua_tointeger(L, 3);
    lua_Integer value = lua_tointeger(L, 4);
    voxel* vox = scripting::level->chunks->get(x, y, z);
    if (vox == nullptr) {
        return 0;
    }
    vox->state.rotation = value;
    scripting::level->chunks->getChunkByVoxel(x, y, z)->setModifiedAndUnsaved();
    return 0;
}

int l_get_block_states(lua_State* L) {
    lua_Integer x = lua_tointeger(L, 1);
    lua_Integer y = lua_tointeger(L, 2);
    lua_Integer z = lua_tointeger(L, 3);
    voxel* vox = scripting::level->chunks->get(x, y, z);
    int states = vox == nullptr ? 0 : blockstate2int(vox->state);
    lua_pushinteger(L, states);
    return 1;
}

int l_set_block_states(lua_State* L) {
    lua_Integer x = lua_tointeger(L, 1);
    lua_Integer y = lua_tointeger(L, 2);
    lua_Integer z = lua_tointeger(L, 3);
    lua_Integer states = lua_tointeger(L, 4);

    Chunk* chunk = scripting::level->chunks->getChunkByVoxel(x, y, z);
    if (chunk == nullptr) {
        return 0;
    }
    voxel* vox = scripting::level->chunks->get(x, y, z);
    vox->state = int2blockstate(states);
    chunk->setModifiedAndUnsaved();
    return 0;
}

int l_get_block_user_bits(lua_State* L) {
    lua_Integer x = lua_tointeger(L, 1);
    lua_Integer y = lua_tointeger(L, 2);
    lua_Integer z = lua_tointeger(L, 3);
    lua_Integer offset = lua_tointeger(L, 4) + VOXEL_USER_BITS_OFFSET;
    lua_Integer bits = lua_tointeger(L, 5);

    voxel* vox = scripting::level->chunks->get(x, y, z);
    if (vox == nullptr) {
        lua_pushinteger(L, 0);
        return 1;
    }
    uint mask = ((1 << bits) - 1) << offset;
    uint data = (blockstate2int(vox->state) & mask) >> offset;
    lua_pushinteger(L, data);
    return 1;
}

int l_set_block_user_bits(lua_State* L) {
    lua_Integer x = lua_tointeger(L, 1);
    lua_Integer y = lua_tointeger(L, 2);
    lua_Integer z = lua_tointeger(L, 3);
    lua_Integer offset = lua_tointeger(L, 4);
    lua_Integer bits = lua_tointeger(L, 5);

    size_t mask = ((1 << bits) - 1) << offset;
    lua_Integer value = (lua_tointeger(L, 6) << offset) & mask;
    
    Chunk* chunk = scripting::level->chunks->getChunkByVoxel(x, y, z);
    if (chunk == nullptr) {
        return 0;
    }
    voxel* vox = scripting::level->chunks->get(x, y, z);
    if (vox == nullptr) {
        return 0;
    }
    vox->state.userbits = (vox->state.userbits & (~mask)) | value;
    chunk->setModifiedAndUnsaved();
    return 0; 
}

int l_is_replaceable_at(lua_State* L) {
    lua_Integer x = lua_tointeger(L, 1);
    lua_Integer y = lua_tointeger(L, 2);
    lua_Integer z = lua_tointeger(L, 3);

    lua_pushboolean(L, scripting::level->chunks->isReplaceableBlock(x, y, z));
    return 1;
}

int l_block_caption(lua_State* L) {
    auto indices = scripting::content->getIndices();
    lua_Integer id = lua_tointeger(L, 1);
    if (static_cast<size_t>(id) >= indices->countBlockDefs()) {
        return 0;
    }
    auto def = indices->getBlockDef(id);
    lua_pushstring(L, def->caption.c_str());
    return 1;
}

const luaL_Reg blocklib [] = {
    {"index", lua_wrap_errors<l_block_index>},
    {"name", lua_wrap_errors<l_block_name>},
    {"material", lua_wrap_errors<l_block_material>},
    {"caption", lua_wrap_errors<l_block_caption>},
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
