#include "api_lua.hpp"
#include "lua_commons.hpp"
#include "LuaState.hpp"
#include "../scripting.hpp"

#include "../../../window/input.hpp"
#include "../../../window/Events.hpp"
#include "../../../util/stringutil.hpp"
#include "../../../frontend/screens/Screen.hpp"
#include "../../../frontend/hud.hpp"
#include "../../../engine.hpp"

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
        throw std::runtime_error("unknown binding "+util::quote(bindname));
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

