#include "api_lua.hpp"

#include "../../../window/input.hpp"
#include "../../../window/Events.hpp"
#include "../../../util/stringutil.hpp"
#include "../../../graphics/ui/GUI.hpp"
#include "../../../frontend/screens/Screen.hpp"
#include "../../../frontend/hud.hpp"
#include "../../../engine.hpp"

namespace scripting {
    extern Hud* hud;
}
using namespace scripting;

static int l_keycode(lua_State* L) {
    auto name = lua::require_string(L, 1);
    return lua::pushinteger(L, static_cast<int>(input_util::keycode_from(name)));
}

static int l_mousecode(lua_State* L) {
    auto name = lua::require_string(L, 1);
    return lua::pushinteger(L, static_cast<int>(input_util::mousecode_from(name)));
}

static int l_add_callback(lua_State* L) {
    auto bindname = lua::require_string(L, 1);
    const auto& bind = Events::bindings.find(bindname);
    if (bind == Events::bindings.end()) {
        throw std::runtime_error("unknown binding "+util::quote(bindname));
    }
    lua::pushvalue(L, 2);
    runnable actual_callback = lua::create_runnable(L);
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
    lua::createtable(L, bindings.size(), 0);

    int i = 0;
    for (auto& entry : bindings) {
        lua::pushstring(L, entry.first);
        lua::rawseti(L, i + 1);
        i++;
    }
    return 1;
}

const luaL_Reg inputlib [] = {
    {"keycode", lua::wrap<l_keycode>},
    {"mousecode", lua::wrap<l_mousecode>},
    {"add_callback", lua::wrap<l_add_callback>},
    {"get_mouse_pos", lua::wrap<l_get_mouse_pos>},
    {"get_bindings", lua::wrap<l_get_bindings>},
    {NULL, NULL}
};

