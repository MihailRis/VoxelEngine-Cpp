#include "libentity.hpp"

static int l_rigidbody_get_vel(lua::State* L) {
    if (auto entity = get_entity(L, 1)) {
        return lua::pushvec3_arr(L, entity->getRigidbody().hitbox.velocity);
    }
    return 0;
}

static int l_rigidbody_set_vel(lua::State* L) {
    if (auto entity = get_entity(L, 1)) {
        entity->getRigidbody().hitbox.velocity = lua::tovec3(L, 2);
    }
    return 0;
}

static int l_rigidbody_is_enabled(lua::State* L) {
    if (auto entity = get_entity(L, 1)) {
        lua::pushboolean(L, entity->getRigidbody().enabled);
    }
    return 0;
}

static int l_rigidbody_set_enabled(lua::State* L) {
    if (auto entity = get_entity(L, 1)) {
        entity->getRigidbody().enabled = lua::toboolean(L, 2);
    }
    return 0;
}

static int l_rigidbody_get_size(lua::State* L) {
    if (auto entity = get_entity(L, 1)) {
        return lua::pushvec3_arr(L, entity->getRigidbody().hitbox.halfsize * 2.0f);
    }
    return 0;
}

static int l_rigidbody_set_size(lua::State* L) {
    if (auto entity = get_entity(L, 1)) {
        entity->getRigidbody().hitbox.halfsize = lua::tovec3(L, 2) * 0.5f;
    }
    return 0;
}

static int l_rigidbody_get_gravity_scale(lua::State* L) {
    if (auto entity = get_entity(L, 1)) {
        return lua::pushnumber(L, entity->getRigidbody().hitbox.gravityScale);
    }
    return 0;
}

static int l_rigidbody_set_gravity_scale(lua::State* L) {
    if (auto entity = get_entity(L, 1)) {
        entity->getRigidbody().hitbox.gravityScale = lua::tonumber(L, 2);
    }
    return 0;
}

const luaL_Reg rigidbodylib [] = {
    {"is_enabled", lua::wrap<l_rigidbody_is_enabled>},
    {"set_enabled", lua::wrap<l_rigidbody_set_enabled>},
    {"get_vel", lua::wrap<l_rigidbody_get_vel>},
    {"set_vel", lua::wrap<l_rigidbody_set_vel>},
    {"get_size", lua::wrap<l_rigidbody_get_size>},
    {"set_size", lua::wrap<l_rigidbody_set_size>},
    {"get_gravity_scale", lua::wrap<l_rigidbody_get_gravity_scale>},
    {"set_gravity_scale", lua::wrap<l_rigidbody_set_gravity_scale>},
    {NULL, NULL}
};
