#include "api_lua.hpp"
#include "lua_util.hpp"
#include "lua_commons.hpp"
#include "LuaState.hpp"
#include "../scripting.hpp"

#include "../../../window/input.hpp"
#include "../../../window/Events.hpp"
#include "../../../util/stringutil.hpp"
#include "../../../graphics/ui/GUI.hpp"
#include "../../../frontend/screens/Screen.hpp"
#include "../../../frontend/hud.hpp"
#include "../../../engine.hpp"

namespace scripting {
    extern lua::LuaState* state;
    extern Hud* hud;
}
using namespace scripting;

static int l_keycode(lua_State* L) {
    auto name = lua::require_string(L, 1);
    lua_pushinteger(L, static_cast<int>(input_util::keycode_from(name)));
    return 1;
}

static int l_mousecode(lua_State* L) {
    auto name = lua::require_string(L, 1);
    lua_pushinteger(L, static_cast<int>(input_util::mousecode_from(name)));
    return 1;
}

static int l_add_callback(lua_State* L) {
    auto bindname = lua::require_string(L, 1);
    const auto& bind = Events::bindings.find(bindname);
    if (bind == Events::bindings.end()) {
        throw std::runtime_error("unknown binding "+util::quote(bindname));
    }
    lua_pushvalue(L, 2);
    runnable actual_callback = state->createRunnable(L);
    runnable callback = [=]() {
        if (!scripting::engine->getGUI()->isFocusCaught()) {
            actual_callback();
        }
    };
    if (hud) {
        hud->keepAlive(bind->second.onactived.add(callback));
    } else {
        engine->keepAlive(bind->second.onactived.add(callback));
    }
    return 0;
}

static int l_get_mouse_pos(lua_State* L) {
    return lua::pushvec2_arr(L, Events::cursor);
}

static int l_get_bindings(lua_State* L) {
    auto& bindings = Events::bindings;
    lua_createtable(L, bindings.size(), 0);

    int i = 0;
    for (auto& entry : bindings) {
        lua_pushstring(L, entry.first.c_str());
        lua_rawseti(L, -2, i + 1);
        i++;
    }
    return 1;
}

const luaL_Reg inputlib [] = {
    {"keycode", lua_wrap_errors<l_keycode>},
    {"mousecode", lua_wrap_errors<l_mousecode>},
    {"add_callback", lua_wrap_errors<l_add_callback>},
    {"get_mouse_pos", lua_wrap_errors<l_get_mouse_pos>},
    {"get_bindings", lua_wrap_errors<l_get_bindings>},
    {NULL, NULL}
};

