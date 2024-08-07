#include <content/Content.hpp>
#include <lighting/Lighting.hpp>
#include <logic/BlocksController.hpp>
#include <logic/LevelController.hpp>
#include <voxels/Block.hpp>
#include <voxels/Chunk.hpp>
#include <voxels/Chunks.hpp>
#include <voxels/voxel.hpp>
#include <world/Level.hpp>
#include "api_lua.hpp"

using namespace scripting;

static const Block* require_block(lua::State* L) {
    auto indices = content->getIndices();
    auto id = lua::tointeger(L, 1);
    return indices->blocks.get(id);
}

static int l_get_def(lua::State* L) {
    if (auto def = require_block(L)) {
        return lua::pushstring(L, def->name);
    }
    return 0;
}

static int l_material(lua::State* L) {
    if (auto def = require_block(L)) {
        return lua::pushstring(L, def->material);
    }
    return 0;
}

static int l_is_solid_at(lua::State* L) {
    auto x = lua::tointeger(L, 1);
    auto y = lua::tointeger(L, 2);
    auto z = lua::tointeger(L, 3);

    return lua::pushboolean(L, level->chunks->isSolidBlock(x, y, z));
}

static int l_count(lua::State* L) {
    return lua::pushinteger(L, indices->blocks.count());
}

static int l_index(lua::State* L) {
    auto name = lua::require_string(L, 1);
    return lua::pushinteger(L, content->blocks.require(name).rt.id);
}

static int l_is_extended(lua::State* L) {
    if (auto def = require_block(L)) {
        return lua::pushboolean(L, def->rt.extended);
    }
    return 0;
}

static int l_get_size(lua::State* L) {
    if (auto def = require_block(L)) {
        return lua::pushivec3_stack(L, def->size.x, def->size.y, def->size.z);
    }
    return 0;
}

static int l_is_segment(lua::State* L) {
    auto x = lua::tointeger(L, 1);
    auto y = lua::tointeger(L, 2);
    auto z = lua::tointeger(L, 3);
    auto vox = level->chunks->get(x, y, z);
    return lua::pushboolean(L, vox->state.segment);
}

static int l_seek_origin(lua::State* L) {
    auto x = lua::tointeger(L, 1);
    auto y = lua::tointeger(L, 2);
    auto z = lua::tointeger(L, 3);
    auto vox = level->chunks->get(x, y, z);
    auto& def = indices->blocks.require(vox->id);
    return lua::pushivec3_stack(
        L, level->chunks->seekOrigin({x, y, z}, def, vox->state)
    );
}

static int l_set(lua::State* L) {
    auto x = lua::tointeger(L, 1);
    auto y = lua::tointeger(L, 2);
    auto z = lua::tointeger(L, 3);
    auto id = lua::tointeger(L, 4);
    auto state = lua::tointeger(L, 5);
    bool noupdate = lua::toboolean(L, 6);
    if (static_cast<size_t>(id) >= indices->blocks.count()) {
        return 0;
    }
    if (!level->chunks->get(x, y, z)) {
        return 0;
    }
    level->chunks->set(x, y, z, id, int2blockstate(state));
    level->lighting->onBlockSet(x, y, z, id);
    if (!noupdate) {
        blocks->updateSides(x, y, z);
    }
    return 0;
}

static int l_get(lua::State* L) {
    auto x = lua::tointeger(L, 1);
    auto y = lua::tointeger(L, 2);
    auto z = lua::tointeger(L, 3);
    auto vox = level->chunks->get(x, y, z);
    int id = vox == nullptr ? -1 : vox->id;
    return lua::pushinteger(L, id);
}

static int l_get_x(lua::State* L) {
    auto x = lua::tointeger(L, 1);
    auto y = lua::tointeger(L, 2);
    auto z = lua::tointeger(L, 3);
    auto vox = level->chunks->get(x, y, z);
    if (vox == nullptr) {
        return lua::pushivec3_stack(L, 1, 0, 0);
    }
    auto& def = level->content->getIndices()->blocks.require(vox->id);
    if (!def.rotatable) {
        return lua::pushivec3_stack(L, 1, 0, 0);
    } else {
        const CoordSystem& rot = def.rotations.variants[vox->state.rotation];
        return lua::pushivec3_stack(L, rot.axisX.x, rot.axisX.y, rot.axisX.z);
    }
}

static int l_get_y(lua::State* L) {
    auto x = lua::tointeger(L, 1);
    auto y = lua::tointeger(L, 2);
    auto z = lua::tointeger(L, 3);
    auto vox = level->chunks->get(x, y, z);
    if (vox == nullptr) {
        return lua::pushivec3_stack(L, 0, 1, 0);
    }
    auto& def = level->content->getIndices()->blocks.require(vox->id);
    if (!def.rotatable) {
        return lua::pushivec3_stack(L, 0, 1, 0);
    } else {
        const CoordSystem& rot = def.rotations.variants[vox->state.rotation];
        return lua::pushivec3_stack(L, rot.axisY.x, rot.axisY.y, rot.axisY.z);
    }
}

static int l_get_z(lua::State* L) {
    auto x = lua::tointeger(L, 1);
    auto y = lua::tointeger(L, 2);
    auto z = lua::tointeger(L, 3);
    auto vox = level->chunks->get(x, y, z);
    if (vox == nullptr) {
        return lua::pushivec3_stack(L, 0, 0, 1);
    }
    auto& def = level->content->getIndices()->blocks.require(vox->id);
    if (!def.rotatable) {
        return lua::pushivec3_stack(L, 0, 0, 1);
    } else {
        const CoordSystem& rot = def.rotations.variants[vox->state.rotation];
        return lua::pushivec3_stack(L, rot.axisZ.x, rot.axisZ.y, rot.axisZ.z);
    }
}

static int l_get_rotation(lua::State* L) {
    auto x = lua::tointeger(L, 1);
    auto y = lua::tointeger(L, 2);
    auto z = lua::tointeger(L, 3);
    voxel* vox = level->chunks->get(x, y, z);
    int rotation = vox == nullptr ? 0 : vox->state.rotation;
    return lua::pushinteger(L, rotation);
}

static int l_set_rotation(lua::State* L) {
    auto x = lua::tointeger(L, 1);
    auto y = lua::tointeger(L, 2);
    auto z = lua::tointeger(L, 3);
    auto value = lua::tointeger(L, 4);
    level->chunks->setRotation(x, y, z, value);
    return 0;
}

static int l_get_states(lua::State* L) {
    auto x = lua::tointeger(L, 1);
    auto y = lua::tointeger(L, 2);
    auto z = lua::tointeger(L, 3);
    auto vox = level->chunks->get(x, y, z);
    int states = vox == nullptr ? 0 : blockstate2int(vox->state);
    return lua::pushinteger(L, states);
}

static int l_set_states(lua::State* L) {
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

static int l_get_user_bits(lua::State* L) {
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

static int l_set_user_bits(lua::State* L) {
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

static int l_is_replaceable_at(lua::State* L) {
    auto x = lua::tointeger(L, 1);
    auto y = lua::tointeger(L, 2);
    auto z = lua::tointeger(L, 3);
    return lua::pushboolean(L, level->chunks->isReplaceableBlock(x, y, z));
}

static int l_caption(lua::State* L) {
    if (auto def = require_block(L)) {
        return lua::pushstring(L, def->caption);
    }
    return 0;
}

static int l_get_textures(lua::State* L) {
    if (auto def = require_block(L)) {
        lua::createtable(L, 6, 0);
        for (size_t i = 0; i < 6; i++) {
            lua::pushstring(L, def->textureFaces[i]);
            lua::rawseti(L, i + 1);
        }
        return 1;
    }
    return 0;
}

static int l_get_model(lua::State* L) {
    if (auto def = require_block(L)) {
        return lua::pushstring(L, to_string(def->model));
    }
    return 0;
}

static int l_get_hitbox(lua::State* L) {
    if (auto def = require_block(L)) {
        auto& hitbox = def->rt.hitboxes[lua::tointeger(L, 2)].at(0);
        lua::createtable(L, 2, 0);

        lua::pushvec3(L, hitbox.min());
        lua::rawseti(L, 1);

        lua::pushvec3(L, hitbox.size());
        lua::rawseti(L, 2);
        return 1;
    }
    return 0;
}

static int l_get_rotation_profile(lua::State* L) {
    if (auto def = require_block(L)) {
        return lua::pushstring(L, def->rotations.name);
    }
    return 0;
}

static int l_get_picking_item(lua::State* L) {
    if (auto def = require_block(L)) {
        return lua::pushinteger(L, def->rt.pickingItem);
    }
    return 0;
}

static int l_place(lua::State* L) {
    auto x = lua::tointeger(L, 1);
    auto y = lua::tointeger(L, 2);
    auto z = lua::tointeger(L, 3);
    auto id = lua::tointeger(L, 4);
    auto state = lua::tointeger(L, 5);
    auto playerid = lua::gettop(L) >= 6 ? lua::tointeger(L, 6) : -1;
    if (static_cast<size_t>(id) >= indices->blocks.count()) {
        return 0;
    }
    if (!level->chunks->get(x, y, z)) {
        return 0;
    }
    const auto def = level->content->getIndices()->blocks.get(id);
    if (def == nullptr) {
        throw std::runtime_error(
            "there is no block with index " + std::to_string(id)
        );
    }
    auto player = level->getObject<Player>(playerid);
    controller->getBlocksController()->placeBlock(
        player ? player.get() : nullptr, *def, int2blockstate(state), x, y, z
    );
    return 0;
}

static int l_destruct(lua::State* L) {
    auto x = lua::tointeger(L, 1);
    auto y = lua::tointeger(L, 2);
    auto z = lua::tointeger(L, 3);
    auto playerid = lua::gettop(L) >= 4 ? lua::tointeger(L, 4) : -1;
    auto voxel = level->chunks->get(x, y, z);
    if (voxel == nullptr) {
        return 0;
    }
    auto& def = level->content->getIndices()->blocks.require(voxel->id);
    auto player = level->getObject<Player>(playerid);
    controller->getBlocksController()->breakBlock(
        player ? player.get() : nullptr, def, x, y, z
    );
    return 0;
}

static int l_raycast(lua::State* L) {
    auto start = lua::tovec<3>(L, 1);
    auto dir = lua::tovec<3>(L, 2);
    auto maxDistance = lua::tonumber(L, 3);
    glm::vec3 end;
    glm::ivec3 normal;
    glm::ivec3 iend;
    if (auto voxel = level->chunks->rayCast(
            start, dir, maxDistance, end, normal, iend
        )) {
        if (lua::gettop(L) >= 4) {
            lua::pushvalue(L, 4);
        } else {
            lua::createtable(L, 0, 5);
        }

        lua::pushvec3(L, end);
        lua::setfield(L, "endpoint");

        lua::pushvec3(L, normal);
        lua::setfield(L, "normal");

        lua::pushnumber(L, glm::distance(start, end));
        lua::setfield(L, "length");

        lua::pushvec3(L, iend);
        lua::setfield(L, "iendpoint");

        lua::pushinteger(L, voxel->id);
        lua::setfield(L, "block");
        return 1;
    }
    return 0;
}

static int l_compose_state(lua::State* L) {
    if (lua::istable(L, 1) || lua::objlen(L, 1) < 3) {
        throw std::runtime_error("expected array of 3 integers");
    }
    blockstate state {};

    lua::rawgeti(L, 1, 1);
    state.rotation = lua::tointeger(L, -1);
    lua::pop(L);
    lua::rawgeti(L, 2, 1);
    state.segment = lua::tointeger(L, -1);
    lua::pop(L);
    lua::rawgeti(L, 3, 1);
    state.userbits = lua::tointeger(L, -1);
    lua::pop(L);

    return lua::pushinteger(L, blockstate2int(state));
}

static int l_decompose_state(lua::State* L) {
    auto stateInt = static_cast<blockstate_t>(lua::tointeger(L, 1));
    auto state = int2blockstate(stateInt);

    lua::createtable(L, 3, 0);
    lua::pushinteger(L, state.rotation);
    lua::rawseti(L, 1);

    lua::pushinteger(L, state.segment);
    lua::rawseti(L, 2);

    lua::pushinteger(L, state.userbits);
    lua::rawseti(L, 3);
    return 1;
}

const luaL_Reg blocklib[] = {
    {"index", lua::wrap<l_index>},
    {"name", lua::wrap<l_get_def>},
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
    {"get_textures", lua::wrap<l_get_textures>},
    {"get_model", lua::wrap<l_get_model>},
    {"get_hitbox", lua::wrap<l_get_hitbox>},
    {"get_rotation_profile", lua::wrap<l_get_rotation_profile>},
    {"get_picking_item", lua::wrap<l_get_picking_item>},
    {"place", lua::wrap<l_place>},
    {"destruct", lua::wrap<l_destruct>},
    {"raycast", lua::wrap<l_raycast>},
    {"compose_state", lua::wrap<l_compose_state>},
    {"decompose_state", lua::wrap<l_decompose_state>},
    {NULL, NULL}};
