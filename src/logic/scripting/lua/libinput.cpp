#include "api_lua.h"
#include "lua_commons.h"
#include "../scripting.h"

#include "../../../window/input.hpp"
#include "../../../window/Events.hpp"
#include "../../../frontend/screens/Screen.hpp"
#include "../../../frontend/hud.h"
#include "../../../engine.h"

#include "LuaState.h"

namespace scripting {
    extern lua::LuaState* state;
    extern Hud* hud;
}

using namespace scripting;

static int l_keycode(lua_State* L) {
    const char* name = lua_tostring(L, 1);
    lua_pushinteger(L, static_cast<int>(input_util::keycode_from(name)));
    return 1;
}

static int l_add_callback(lua_State* L) {
    auto bindname = lua_tostring(L, 1);
    const auto& bind = Events::bindings.find(bindname);
    if (bind == Events::bindings.end()) {
        luaL_error(L, "unknown binding %q", bindname);
    }
    state->pushvalue(2);
    runnable callback = state->createRunnable();
    if (hud) {
        hud->keepAlive(bind->second.onactived.add(callback));
    } else {
        engine->keepAlive(bind->second.onactived.add(callback));
    }
    return 0;
}

const luaL_Reg inputlib [] = {
    {"keycode", lua_wrap_errors<l_keycode>},
    {"add_callback", lua_wrap_errors<l_add_callback>},
    {NULL, NULL}
};

