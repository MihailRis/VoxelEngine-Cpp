#include "api_lua.hpp"

#include "../../../world/Level.hpp"
#include "../../../voxels/Chunks.hpp"
#include "../../../voxels/Chunk.hpp"
#include "../../../voxels/Block.hpp"
#include "../../../voxels/voxel.hpp"
#include "../../../lighting/Lighting.hpp"
#include "../../../content/Content.hpp"
#include "../../../logic/BlocksController.hpp"

using namespace scripting;

static Block* require_block(lua_State* L) {
    auto indices = content->getIndices();
    auto id = lua::tointeger(L, 1);
    if (static_cast<size_t>(id) >= indices->countBlockDefs()) {
        return nullptr;
    }
    return indices->getBlockDef(id);
}

static int l_name(lua_State* L) {
    if (auto def = require_block(L)) {
        return lua::pushstring(L, def->name);
    }
    return 0;
}

static int l_material(lua_State* L) {
    if (auto def = require_block(L)) {
        return lua::pushstring(L, def->material);
    }
    return 0;
}

static int l_is_solid_at(lua_State* L) {
    auto x = lua::tointeger(L, 1);
    auto y = lua::tointeger(L, 2);
    auto z = lua::tointeger(L, 3);

    return lua::pushboolean(L, level->chunks->isSolidBlock(x, y, z));
}

static int l_count(lua_State* L) {
    return lua::pushinteger(L, indices->countBlockDefs());
}

static int l_index(lua_State* L) {
    auto name = lua::require_string(L, 1);
    return lua::pushinteger(L, content->requireBlock(name).rt.id);
}

static int l_is_extended(lua_State* L) {
    if (auto def = require_block(L)) {
        return lua::pushboolean(L, def->rt.extended);
    }
    return 0;
}

static int l_get_size(lua_State* L) {
    if (auto def = require_block(L)) {
        return lua::pushivec3(L, def->size.x, def->size.y, def->size.z);
    }
    return 0;
}

static int l_is_segment(lua_State* L) {
    auto x = lua::tointeger(L, 1);
    auto y = lua::tointeger(L, 2);
    auto z = lua::tointeger(L, 3);
    auto vox = level->chunks->get(x, y, z);
    return lua::pushboolean(L, vox->state.segment);
}

static int l_seek_origin(lua_State* L) {
    auto x = lua::tointeger(L, 1);
    auto y = lua::tointeger(L, 2);
    auto z = lua::tointeger(L, 3);
    auto vox = level->chunks->get(x, y, z);
    auto def = indices->getBlockDef(vox->id);
    return lua::pushivec3(L, level->chunks->seekOrigin({x, y, z}, def, vox->state));
}

static int l_set(lua_State* L) {
    auto x = lua::tointeger(L, 1);
    auto y = lua::tointeger(L, 2);
    auto z = lua::tointeger(L, 3);
    auto id = lua::tointeger(L, 4);    
    auto state = lua::tointeger(L, 5);
    bool noupdate = lua::toboolean(L, 6);
    if (static_cast<size_t>(id) >= indices->countBlockDefs()) {
        return 0;
    }
    if (!level->chunks->get(x, y, z)) {
        return 0;
    }
    level->chunks->set(x, y, z, id, int2blockstate(state));
    level->lighting->onBlockSet(x,y,z, id);
    if (!noupdate) {
        blocks->updateSides(x, y, z);
    }
    return 0;
}

static int l_get(lua_State* L) {
    auto x = lua::tointeger(L, 1);
    auto y = lua::tointeger(L, 2);
    auto z = lua::tointeger(L, 3);
    auto vox = level->chunks->get(x, y, z);
    int id = vox == nullptr ? -1 : vox->id;
    return lua::pushinteger(L, id);
}

static int l_get_x(lua_State* L) {
    auto x = lua::tointeger(L, 1);
    auto y = lua::tointeger(L, 2);
    auto z = lua::tointeger(L, 3);
    auto vox = level->chunks->get(x, y, z);
    if (vox == nullptr) {
        return lua::pushivec3(L, 1, 0, 0);
    }
    auto def = level->content->getIndices()->getBlockDef(vox->id);
    if (!def->rotatable) {
        return lua::pushivec3(L, 1, 0, 0);
    } else {
        const CoordSystem& rot = def->rotations.variants[vox->state.rotation];
        return lua::pushivec3(L, rot.axisX.x, rot.axisX.y, rot.axisX.z);
    }
}

static int l_get_y(lua_State* L) {
    auto x = lua::tointeger(L, 1);
    auto y = lua::tointeger(L, 2);
    auto z = lua::tointeger(L, 3);
    auto vox = level->chunks->get(x, y, z);
    if (vox == nullptr) {
        return lua::pushivec3(L, 0, 1, 0);
    }
    auto def = level->content->getIndices()->getBlockDef(vox->id);
    if (!def->rotatable) {
        return lua::pushivec3(L, 0, 1, 0);
    } else {
        const CoordSystem& rot = def->rotations.variants[vox->state.rotation];
        return lua::pushivec3(L, rot.axisY.x, rot.axisY.y, rot.axisY.z);
    }
}

static int l_get_z(lua_State* L) {
    auto x = lua::tointeger(L, 1);
    auto y = lua::tointeger(L, 2);
    auto z = lua::tointeger(L, 3);
    auto vox = level->chunks->get(x, y, z);
    if (vox == nullptr) {
        return lua::pushivec3(L, 0, 0, 1);
    }
    auto def = level->content->getIndices()->getBlockDef(vox->id);
    if (!def->rotatable) {
        return lua::pushivec3(L, 0, 0, 1);
    } else {
        const CoordSystem& rot = def->rotations.variants[vox->state.rotation];
        return lua::pushivec3(L, rot.axisZ.x, rot.axisZ.y, rot.axisZ.z);
    }
}

static int l_get_rotation(lua_State* L) {
    auto x = lua::tointeger(L, 1);
    auto y = lua::tointeger(L, 2);
    auto z = lua::tointeger(L, 3);
    voxel* vox = level->chunks->get(x, y, z);
    int rotation = vox == nullptr ? 0 : vox->state.rotation;
    return lua::pushinteger(L, rotation);
}

static int l_set_rotation(lua_State* L) {
    auto x = lua::tointeger(L, 1);
    auto y = lua::tointeger(L, 2);
    auto z = lua::tointeger(L, 3);
    auto value = lua::tointeger(L, 4);
    level->chunks->setRotation(x, y, z, value);
    return 0;
}

static int l_get_states(lua_State* L) {
    auto x = lua::tointeger(L, 1);
    auto y = lua::tointeger(L, 2);
    auto z = lua::tointeger(L, 3);
    auto vox = level->chunks->get(x, y, z);
    int states = vox == nullptr ? 0 : blockstate2int(vox->state);
    return lua::pushinteger(L, states);
}

static int l_set_states(lua_State* L) {
    auto x = lua::tointeger(L, 1);
    auto y = lua::tointeger(L, 2);
    auto z = lua::tointeger(L, 3);
    auto states = lua::tointeger(L, 4);

    auto chunk = level->chunks->getChunkByVoxel(x, y, z);
    if (chunk == nullptr) {
        return 0;
    }
    auto vox = level->chunks->get(x, y, z);
    vox->state = int2blockstate(states);
    chunk->setModifiedAndUnsaved();
    return 0;
}

static int l_get_user_bits(lua_State* L) {
    auto x = lua::tointeger(L, 1);
    auto y = lua::tointeger(L, 2);
    auto z = lua::tointeger(L, 3);
    auto offset = lua::tointeger(L, 4) + VOXEL_USER_BITS_OFFSET;
    auto bits = lua::tointeger(L, 5);

    auto vox = level->chunks->get(x, y, z);
    if (vox == nullptr) {
        return lua::pushinteger(L, 0);
    }
    uint mask = ((1 << bits) - 1) << offset;
    uint data = (blockstate2int(vox->state) & mask) >> offset;
    return lua::pushinteger(L, data);
}

static int l_set_user_bits(lua_State* L) {
    auto x = lua::tointeger(L, 1);
    auto y = lua::tointeger(L, 2);
    auto z = lua::tointeger(L, 3);
    auto offset = lua::tointeger(L, 4);
    auto bits = lua::tointeger(L, 5);

    size_t mask = ((1 << bits) - 1) << offset;
    auto value = (lua::tointeger(L, 6) << offset) & mask;
    
    auto chunk = level->chunks->getChunkByVoxel(x, y, z);
    if (chunk == nullptr) {
        return 0;
    }
    auto vox = level->chunks->get(x, y, z);
    if (vox == nullptr) {
        return 0;
    }
    vox->state.userbits = (vox->state.userbits & (~mask)) | value;
    chunk->setModifiedAndUnsaved();
    return 0; 
}

static int l_is_replaceable_at(lua_State* L) {
    auto x = lua::tointeger(L, 1);
    auto y = lua::tointeger(L, 2);
    auto z = lua::tointeger(L, 3);
    return lua::pushboolean(L, level->chunks->isReplaceableBlock(x, y, z));
}

static int l_caption(lua_State* L) {
    if (auto def = require_block(L)) {
        return lua::pushstring(L, def->caption);
    }
    return 0;
}

const luaL_Reg blocklib [] = {
    {"index", lua::wrap<l_index>},
    {"name", lua::wrap<l_name>},
    {"material", lua::wrap<l_material>},
    {"caption", lua::wrap<l_caption>},
    {"defs_count", lua::wrap<l_count>},
    {"is_solid_at", lua::wrap<l_is_solid_at>},
    {"is_replaceable_at", lua::wrap<l_is_replaceable_at>},
    {"set", lua::wrap<l_set>},
    {"get", lua::wrap<l_get>},
    {"get_X", lua::wrap<l_get_x>},
    {"get_Y", lua::wrap<l_get_y>},
    {"get_Z", lua::wrap<l_get_z>},
    {"get_states", lua::wrap<l_get_states>},
    {"set_states", lua::wrap<l_set_states>},
    {"get_rotation", lua::wrap<l_get_rotation>},
    {"set_rotation", lua::wrap<l_set_rotation>},
    {"get_user_bits", lua::wrap<l_get_user_bits>},
    {"set_user_bits", lua::wrap<l_set_user_bits>},
    {"is_extended", lua::wrap<l_is_extended>},
    {"get_size", lua::wrap<l_get_size>},
    {"is_segment", lua::wrap<l_is_segment>},
    {"seek_origin", lua::wrap<l_seek_origin>},
    {NULL, NULL}
};
