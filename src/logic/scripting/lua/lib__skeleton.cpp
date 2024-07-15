#include "libentity.hpp"

#include "../../../objects/rigging.hpp"

static int index_range_check(const rigging::Skeleton& skeleton, lua::Integer index) {
    if (static_cast<size_t>(index) >= skeleton.pose.matrices.size()) {
        throw std::runtime_error("index out of range [0, " +
                                 std::to_string(skeleton.pose.matrices.size()) +
                                 "]");
    }
    return static_cast<int>(index);
}

static int l_get_model(lua::State* L) {
    if (auto entity = get_entity(L, 1)) {
        auto& skeleton = entity->getSkeleton();
        auto* rigConfig = skeleton.config;
        auto index = index_range_check(skeleton, lua::tointeger(L, 2));
        return lua::pushstring(L, rigConfig->getNodes()[index]->getModelName());
    }
    return 0;
}

static int l_get_matrix(lua::State* L) {
    if (auto entity = get_entity(L, 1)) {
        auto& skeleton = entity->getSkeleton();
        auto index = index_range_check(skeleton, lua::tointeger(L, 2));
        return lua::pushmat4(L, skeleton.pose.matrices[index]);
    }
    return 0;
}

static int l_set_matrix(lua::State* L) {
    if (auto entity = get_entity(L, 1)) {
        auto& skeleton = entity->getSkeleton();
        auto index = index_range_check(skeleton, lua::tointeger(L, 2));
        skeleton.pose.matrices[index] = lua::tomat4(L, 3);
    }
    return 0;
}

static int l_get_texture(lua::State* L) {
    if (auto entity = get_entity(L, 1)) {
        auto& skeleton = entity->getSkeleton();
        skeleton.textures[lua::require_string(L, 2)] = lua::require_string(L, 3);
        const auto& found = skeleton.textures.find(lua::require_string(L, 2));
        if (found != skeleton.textures.end()) {
            return lua::pushstring(L, found->second);
        }
    }
    return 0;
}

static int l_set_texture(lua::State* L) {
    if (auto entity = get_entity(L, 1)) {
        auto& skeleton = entity->getSkeleton();
        skeleton.textures[lua::require_string(L, 2)] = lua::require_string(L, 3);
    }
    return 0;
}

static int l_index(lua::State* L) {
    if (auto entity = get_entity(L, 1)) {
        auto& skeleton = entity->getSkeleton();
        if (auto bone = skeleton.config->find(lua::require_string(L, 2))) {
            return lua::tointeger(L, bone->getIndex());
        }
    }
    return 0;
}

const luaL_Reg skeletonlib [] = {
    {"get_model", lua::wrap<l_get_model>},
    {"get_matrix", lua::wrap<l_get_matrix>},
    {"set_matrix", lua::wrap<l_set_matrix>},
    {"get_texture", lua::wrap<l_get_texture>},
    {"set_texture", lua::wrap<l_set_texture>},
    {"index", lua::wrap<l_index>},
    {NULL, NULL}
};
