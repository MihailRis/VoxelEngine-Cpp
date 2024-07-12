#include "libentity.hpp"

#include "../../../util/stringutil.hpp"

static int l_get_vel(lua::State* L) {
    if (auto entity = get_entity(L, 1)) {
        return lua::pushvec3_arr(L, entity->getRigidbody().hitbox.velocity);
    }
    return 0;
}

static int l_set_vel(lua::State* L) {
    if (auto entity = get_entity(L, 1)) {
        entity->getRigidbody().hitbox.velocity = lua::tovec3(L, 2);
    }
    return 0;
}

static int l_is_enabled(lua::State* L) {
    if (auto entity = get_entity(L, 1)) {
        lua::pushboolean(L, entity->getRigidbody().enabled);
    }
    return 0;
}

static int l_set_enabled(lua::State* L) {
    if (auto entity = get_entity(L, 1)) {
        entity->getRigidbody().enabled = lua::toboolean(L, 2);
    }
    return 0;
}

static int l_get_size(lua::State* L) {
    if (auto entity = get_entity(L, 1)) {
        return lua::pushvec3_arr(L, entity->getRigidbody().hitbox.halfsize * 2.0f);
    }
    return 0;
}

static int l_set_size(lua::State* L) {
    if (auto entity = get_entity(L, 1)) {
        entity->getRigidbody().hitbox.halfsize = lua::tovec3(L, 2) * 0.5f;
    }
    return 0;
}

static int l_get_gravity_scale(lua::State* L) {
    if (auto entity = get_entity(L, 1)) {
        return lua::pushnumber(L, entity->getRigidbody().hitbox.gravityScale);
    }
    return 0;
}

static int l_set_gravity_scale(lua::State* L) {
    if (auto entity = get_entity(L, 1)) {
        entity->getRigidbody().hitbox.gravityScale = lua::tonumber(L, 2);
    }
    return 0;
}

static int l_is_vdamping(lua::State* L) {
    if (auto entity = get_entity(L, 1)) {
        return lua::pushboolean(L, entity->getRigidbody().hitbox.verticalDamping);
    }
    return 0;
}

static int l_set_vdamping(lua::State* L) {
    if (auto entity = get_entity(L, 1)) {
        entity->getRigidbody().hitbox.verticalDamping = lua::toboolean(L, 2);
    }
    return 0;
}

static int l_is_grounded(lua::State* L) {
    if (auto entity = get_entity(L, 1)) {
        return lua::pushboolean(L, entity->getRigidbody().hitbox.grounded);
    }
    return 0;
}

static int l_is_crouching(lua::State* L) {
    if (auto entity = get_entity(L, 1)) {
        return lua::pushboolean(L, entity->getRigidbody().hitbox.crouching);
    }
    return 0;
}

static int l_set_crouching(lua::State* L) {
    if (auto entity = get_entity(L, 1)) {
        entity->getRigidbody().hitbox.crouching = lua::toboolean(L, 2);
    }
    return 0;
}

static int l_get_body_type(lua::State* L) {
    if (auto entity = get_entity(L, 1)) {
        return lua::pushstring(L, to_string(entity->getRigidbody().hitbox.type));
    }
    return 0;
}

static int l_set_body_type(lua::State* L) {
    if (auto entity = get_entity(L, 1)) {
        if (auto type = BodyType_from(lua::tostring(L, 2))) {
            entity->getRigidbody().hitbox.type = *type;
        } else {
            throw std::runtime_error(
                "unknown body type "+util::quote(lua::tostring(L, 2)));
        }
    }
    return 0;
}

static int l_get_linear_damping(lua::State* L) {
    if (auto entity = get_entity(L, 1)) {
        return lua::pushnumber(L, entity->getRigidbody().hitbox.linearDamping);
    }
    return 0;
}

static int l_set_linear_damping(lua::State* L) {
    if (auto entity = get_entity(L, 1)) {
        entity->getRigidbody().hitbox.linearDamping = lua::tonumber(L, 2);
    }
    return 0;
}

const luaL_Reg rigidbodylib [] = {
    {"is_enabled", lua::wrap<l_is_enabled>},
    {"set_enabled", lua::wrap<l_set_enabled>},
    {"get_vel", lua::wrap<l_get_vel>},
    {"set_vel", lua::wrap<l_set_vel>},
    {"get_size", lua::wrap<l_get_size>},
    {"set_size", lua::wrap<l_set_size>},
    {"get_gravity_scale", lua::wrap<l_get_gravity_scale>},
    {"set_gravity_scale", lua::wrap<l_set_gravity_scale>},
    {"get_linear_damping", lua::wrap<l_get_linear_damping>},
    {"set_linear_damping", lua::wrap<l_set_linear_damping>},
    {"is_vdamping", lua::wrap<l_is_vdamping>},
    {"set_vdamping", lua::wrap<l_set_vdamping>},
    {"is_grounded", lua::wrap<l_is_grounded>},
    {"is_crouching", lua::wrap<l_is_crouching>},
    {"set_crouching", lua::wrap<l_set_crouching>},
    {"get_body_type", lua::wrap<l_get_body_type>},
    {"set_body_type", lua::wrap<l_set_body_type>},
    {NULL, NULL}
};
