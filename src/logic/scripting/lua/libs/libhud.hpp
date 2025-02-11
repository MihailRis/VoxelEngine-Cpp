#pragma once

#include "api_lua.hpp"

#include "logic/scripting/scripting_hud.hpp"
#include "graphics/render/WorldRenderer.hpp"
#include "engine/Engine.hpp"

template <lua_CFunction func>
inline int wrap_hud(lua_State* L) {
    if (scripting::hud == nullptr) {
        return luaL_error(
            L, "renderer is not initialized yet, see hud.lua on_hud_open event"
        );
    }
    return lua::wrap<func>(L);
}
