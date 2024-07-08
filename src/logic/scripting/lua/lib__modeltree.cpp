#include "libentity.hpp"

#include "../../../objects/rigging.hpp"

static int index_range_check(const rigging::Rig& rig, lua::Integer index) {
    if (static_cast<size_t>(index) >= rig.pose.matrices.size()) {
        throw std::runtime_error("index out of range [0, " +
                                 std::to_string(rig.pose.matrices.size()) +
                                 "]");
    }
    return static_cast<int>(index);
}

static int l_modeltree_get_model(lua::State* L) {
    if (auto entity = get_entity(L, 1)) {
        auto& rig = entity->getModeltree();
        auto* rigConfig = rig.config;
        auto index = index_range_check(rig, lua::tointeger(L, 2));
        return lua::pushstring(L, rigConfig->getNodes()[index]->getModelName());
    }
    return 0;
}

static int l_modeltree_get_matrix(lua::State* L) {
    if (auto entity = get_entity(L, 1)) {
        auto& rig = entity->getModeltree();
        auto index = index_range_check(rig, lua::tointeger(L, 2));
        return lua::pushmat4(L, rig.pose.matrices[index]);
    }
    return 0;
}

static int l_modeltree_set_matrix(lua::State* L) {
    if (auto entity = get_entity(L, 1)) {
        auto& rig = entity->getModeltree();
        auto index = index_range_check(rig, lua::tointeger(L, 2));
        rig.pose.matrices[index] = lua::tomat4(L, 3);
    }
    return 0;
}

static int l_modeltree_set_texture(lua::State* L) {
    if (auto entity = get_entity(L, 1)) {
        auto& rig = entity->getModeltree();
        rig.textures[lua::require_string(L, 2)] = lua::require_string(L, 3);
    }
    return 0;
}

const luaL_Reg modeltreelib [] = {
    {"get_model", lua::wrap<l_modeltree_get_model>},
    {"get_matrix", lua::wrap<l_modeltree_get_matrix>},
    {"set_matrix", lua::wrap<l_modeltree_set_matrix>},
    {"set_texture", lua::wrap<l_modeltree_set_texture>},
    {NULL, NULL}
};
