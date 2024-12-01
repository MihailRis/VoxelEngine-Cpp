#include "api_lua.hpp"

#include "logic/scripting/scripting_hud.hpp"
#include "graphics/render/WorldRenderer.hpp"
#include "graphics/render/BlockWrapsRenderer.hpp"

using namespace scripting;

static int l_wrap(lua::State* L) {
    auto position = lua::tovec3(L, 1);
    std::string texture = lua::require_string(L, 2);

    return lua::pushinteger(
        L, renderer->blockWraps->add(position, std::move(texture))
    );
}

static int l_unwrap(lua::State* L) {
    renderer->blockWraps->remove(lua::tointeger(L, 1));
    return 0;
}

static int l_set_pos(lua::State* L) {
    if (auto wrapper = renderer->blockWraps->get(lua::tointeger(L, 1))) {
        wrapper->position = lua::tovec3(L, 2);
    }
    return 0;
}

static int l_set_texture(lua::State* L) {
    if (auto wrapper = renderer->blockWraps->get(lua::tointeger(L, 1))) {
        wrapper->texture = lua::require_string(L, 2);
    }
    return 0;
}

static int l_get_pos(lua::State* L) {
    if (const auto wrapper = renderer->blockWraps->get(lua::tointeger(L, 1))) {
        return lua::pushvec3(L, wrapper->position);
    }
    return 0;
}

static int l_get_texture(lua::State* L) {
    if (const auto wrapper = renderer->blockWraps->get(lua::tointeger(L, 1))) {
        return lua::pushstring(L, wrapper->texture);
    }
    return 0;
}

static int l_is_alive(lua::State* L) {
    return lua::pushboolean(
        L, renderer->blockWraps->get(lua::tointeger(L, 1)) != nullptr
    );
}

static int l_get_on_pos(lua::State* L) {
    if (const auto id = renderer->blockWraps->get_id_by_pos(lua::tovec3(L, 1))) {
        return lua::pushinteger(L, id);
    }
    return 0;
}

const luaL_Reg blockwrapslib[] = {
    {"wrap", lua::wrap<l_wrap>},
    {"unwrap", lua::wrap<l_unwrap>},
    {"set_pos", lua::wrap<l_set_pos>},
    {"set_texture", lua::wrap<l_set_texture>},
    {"get_pos", lua::wrap<l_get_pos>},
    {"get_texture", lua::wrap<l_get_texture>},
    {"is_alive", lua::wrap<l_is_alive>},
    {"get_on_pos", lua::wrap<l_get_on_pos>},
    {NULL, NULL}
};
