#include "libentity.hpp"

#include "content/Content.hpp"
#include "engine.hpp"
#include "objects/Entities.hpp"
#include "objects/EntityDef.hpp"
#include "objects/Player.hpp"
#include "objects/rigging.hpp"
#include "physics/Hitbox.hpp"
#include "voxels/Chunks.hpp"
#include "voxels/Block.hpp"
#include "voxels/blocks_agent.hpp"
#include "window/Camera.hpp"

using namespace scripting;

static const EntityDef* require_entity_def(lua::State* L) {
    auto indices = content->getIndices();
    auto id = lua::tointeger(L, 1);
    return indices->entities.get(id);
}

static int l_exists(lua::State* L) {
    return lua::pushboolean(L, get_entity(L, 1).has_value());
}

static int l_def_index(lua::State* L) {
    auto name = lua::require_string(L, 1);
    return lua::pushinteger(L, content->entities.require(name).rt.id);
}

static int l_def_name(lua::State* L) {
    if (auto def = require_entity_def(L)) {
        return lua::pushstring(L, def->name);
    }
    return 0;
}

static int l_def_hitbox(lua::State* L) {
    if (auto def = require_entity_def(L)) {
        return lua::pushvec(L, def->hitbox);
    }
    return 0;
}

static int l_defs_count(lua::State* L) {
    return lua::pushinteger(L, indices->entities.count());
}

static int l_get_def(lua::State* L) {
    if (auto entity = get_entity(L, 1)) {
        return lua::pushinteger(L, entity->getDef().rt.id);
    }
    return 0;
}

static int l_show(lua::State* L) {
    auto level = controller->getLevel();
    auto defname = lua::tostring(L, 1);
    auto& def = content->entities.require(defname);
    auto pos = lua::tovec3(L, 2);
    dv::value args = nullptr;
    if (lua::gettop(L) > 2) {
        args = lua::tovalue(L, 3);
    }
    level->entities->spawn(def, pos, std::move(args));
    return 1;
}

static int l_despawn(lua::State* L) {
    if (auto entity = get_entity(L, 1)) {
        entity->destroy();
    }
    return 0;
}

static int l_get_skeleton(lua::State* L) {
    if (auto entity = get_entity(L, 1)) {
        return lua::pushstring(L, entity->getSkeleton().config->getName());
    }
    return 0;
}

static int l_set_skeleton(lua::State* L) {
    if (auto entity = get_entity(L, 1)) {
        std::string skeletonName = lua::require_string(L, 2);
        auto rigConfig = content->getSkeleton(skeletonName);
        if (rigConfig == nullptr) {
            throw std::runtime_error(
                "skeleton not found '" + skeletonName + "'"
            );
        }
        entity->setRig(rigConfig);
    }
    return 0;
}

static int l_get_all_in_box(lua::State* L) {
    auto pos = lua::tovec<3>(L, 1);
    auto size = lua::tovec<3>(L, 2);
    auto found = level->entities->getAllInside(AABB(pos, pos + size));
    lua::createtable(L, found.size(), 0);
    for (size_t i = 0; i < found.size(); i++) {
        const auto& entity = found[i];
        lua::pushinteger(L, entity.getUID());
        lua::rawseti(L, i + 1);
    }
    return 1;
}

static int l_get_all_in_radius(lua::State* L) {
    auto pos = lua::tovec<3>(L, 1);
    auto radius = lua::tonumber(L, 2);
    auto found = level->entities->getAllInRadius(pos, radius);
    lua::createtable(L, found.size(), 0);
    for (size_t i = 0; i < found.size(); i++) {
        const auto& entity = found[i];
        lua::pushinteger(L, entity.getUID());
        lua::rawseti(L, i + 1);
    }
    return 1;
}

static int l_raycast(lua::State* L) {
    auto start = lua::tovec<3>(L, 1);
    auto dir = lua::tovec<3>(L, 2);
    auto maxDistance = lua::tonumber(L, 3);
    auto ignoreEntityId = lua::tointeger(L, 4);
    std::set<blockid_t> filteredBlocks {};
    if (lua::gettop(L) >= 6) {
        if (lua::istable(L, 6)) {
            int addLen = lua::objlen(L, 6);
            for (int i = 0; i < addLen; i++) {
                lua::rawgeti(L, i + 1, 6);
                auto blockName = std::string(lua::tostring(L, -1));
                const Block* block = content->blocks.find(blockName);
                if (block != nullptr) {
                    filteredBlocks.insert(block->rt.id);
                }
                lua::pop(L);
            }
        } else {
            throw std::runtime_error("table expected for filter");
        }
    }

    glm::vec3 end;
    glm::ivec3 normal;
    glm::ivec3 iend;

    blockid_t block = BLOCK_VOID;

    if (auto voxel = blocks_agent::raycast(
            *level->chunks,
            start,
            dir,
            maxDistance,
            end,
            normal,
            iend,
            filteredBlocks
        )) {
        maxDistance = glm::distance(start, end);
        block = voxel->id;
    }
    if (auto ray =
            level->entities->rayCast(start, dir, maxDistance, ignoreEntityId)) {
        if (lua::gettop(L) >= 5 && !lua::isnil(L, 5)) {
            lua::pushvalue(L, 5);
        } else {
            lua::createtable(L, 0, 6);
        }

        lua::pushvec3(L, start + dir * ray->distance);
        lua::setfield(L, "endpoint");

        lua::pushvec3(L, ray->normal);
        lua::setfield(L, "normal");

        lua::pushnumber(L, glm::distance(start, end));
        lua::setfield(L, "length");

        lua::pushvec3(L, iend);
        lua::setfield(L, "iendpoint");

        lua::pushinteger(L, block);
        lua::setfield(L, "block");

        lua::pushinteger(L, ray->entity);
        lua::setfield(L, "entity");
        return 1;
    } else if (block != BLOCK_VOID) {
        if (lua::gettop(L) >= 5 && !lua::isnil(L, 5)) {
            lua::pushvalue(L, 5);
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

        lua::pushinteger(L, block);
        lua::setfield(L, "block");
        return 1;
    }
    return 0;
}

const luaL_Reg entitylib[] = {
    {"exists", lua::wrap<l_exists>},
    {"def_index", lua::wrap<l_def_index>},
    {"def_name", lua::wrap<l_def_name>},
    {"def_hitbox", lua::wrap<l_def_hitbox>},
    {"get_def", lua::wrap<l_get_def>},
    {"defs_count", lua::wrap<l_defs_count>},
    {"spawn", lua::wrap<l_show>},
    {"despawn", lua::wrap<l_despawn>},
    {"get_skeleton", lua::wrap<l_get_skeleton>},
    {"set_skeleton", lua::wrap<l_set_skeleton>},
    {"get_all_in_box", lua::wrap<l_get_all_in_box>},
    {"get_all_in_radius", lua::wrap<l_get_all_in_radius>},
    {"raycast", lua::wrap<l_raycast>},
    {NULL, NULL}
};
