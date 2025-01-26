#include "objects/rigging.hpp"
#include "libentity.hpp"

static int index_range_check(
    const rigging::Skeleton& skeleton, lua::Integer index
) {
    if (static_cast<size_t>(index) >= skeleton.pose.matrices.size()) {
        throw std::runtime_error(
            "index out of range [0, " +
            std::to_string(skeleton.pose.matrices.size()) + "]"
        );
    }
    return static_cast<int>(index);
}

static int l_get_model(lua::State* L) {
    if (auto entity = get_entity(L, 1)) {
        auto& skeleton = entity->getSkeleton();
        auto* rigConfig = skeleton.config;
        auto index = index_range_check(skeleton, lua::tointeger(L, 2));
        const auto& modelOverride = skeleton.modelOverrides[index];
        if (!modelOverride.model) {
            return lua::pushstring(L, modelOverride.name);
        }
        return lua::pushstring(L, rigConfig->getBones()[index]->model.name);
    }
    return 0;
}

static int l_set_model(lua::State* L) {
    if (auto entity = get_entity(L, 1)) {
        auto& skeleton = entity->getSkeleton();
        auto index = index_range_check(skeleton, lua::tointeger(L, 2));
        auto& modelOverride = skeleton.modelOverrides[index];
        if (lua::isnoneornil(L, 3)) {
            modelOverride = {"", nullptr, true};
        } else {
            modelOverride = {lua::require_string(L, 3), nullptr, true};
        }
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
        skeleton.textures[lua::require_string(L, 2)] =
            lua::require_string(L, 3);
    }
    return 0;
}

static int l_index(lua::State* L) {
    if (auto entity = get_entity(L, 1)) {
        auto& skeleton = entity->getSkeleton();
        if (auto bone = skeleton.config->find(lua::require_string(L, 2))) {
            return lua::pushinteger(L, bone->getIndex());
        }
    }
    return 0;
}

static int l_is_visible(lua::State* L) {
    if (auto entity = get_entity(L, 1)) {
        auto& skeleton = entity->getSkeleton();
        if (!lua::isnoneornil(L, 2)) {
            auto index = index_range_check(skeleton, lua::tointeger(L, 2));
            return lua::pushboolean(L, skeleton.flags.at(index).visible);
        }
        return lua::pushboolean(L, skeleton.visible);
    }
    return 0;
}

static int l_set_visible(lua::State* L) {
    if (auto entity = get_entity(L, 1)) {
        auto& skeleton = entity->getSkeleton();
        if (!lua::isnoneornil(L, 3)) {
            auto index = index_range_check(skeleton, lua::tointeger(L, 2));
            skeleton.flags.at(index).visible = lua::toboolean(L, 3);
        } else {
            skeleton.visible = lua::toboolean(L, 2);
        }
    }
    return 0;
}

static int l_get_color(lua::State* L) {
    if (auto entity = get_entity(L, 1)) {
        auto& skeleton = entity->getSkeleton();
        return lua::pushvec(L, skeleton.tint);
    }
    return 0;
}

static int l_set_color(lua::State* L) {
    if (auto entity = get_entity(L, 1)) {
        auto& skeleton = entity->getSkeleton();
        skeleton.tint = lua::tovec3(L, 2);
    }
    return 0;
}

static int l_is_interpolated(lua::State* L) {
    if (auto entity = get_entity(L, 1)) {
        auto& skeleton = entity->getSkeleton();
        return lua::pushboolean(L, skeleton.interpolation.isEnabled());
    }
    return 0;
}

static int l_set_interpolated(lua::State* L) {
    if (auto entity = get_entity(L, 1)) {
        auto& skeleton = entity->getSkeleton();
        skeleton.interpolation.setEnabled(lua::toboolean(L, 2));
    }
    return 0;
}

const luaL_Reg skeletonlib[] = {
    {"get_model", lua::wrap<l_get_model>},
    {"set_model", lua::wrap<l_set_model>},
    {"get_matrix", lua::wrap<l_get_matrix>},
    {"set_matrix", lua::wrap<l_set_matrix>},
    {"get_texture", lua::wrap<l_get_texture>},
    {"set_texture", lua::wrap<l_set_texture>},
    {"index", lua::wrap<l_index>},
    {"is_visible", lua::wrap<l_is_visible>},
    {"set_visible", lua::wrap<l_set_visible>},
    {"get_color", lua::wrap<l_get_color>},
    {"set_color", lua::wrap<l_set_color>},
    {"is_interpolated", lua::wrap<l_is_interpolated>},
    {"set_interpolated", lua::wrap<l_set_interpolated>},
    {NULL, NULL}};
