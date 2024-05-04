#include "api_lua.h"
#include "lua_commons.h"
#include "../scripting.h"

#include "../../../window/input.h"
#include "../../../window/Events.h"
#include "../../../frontend/screens/Screen.hpp"
#include "../../../engine.h"

#include "LuaState.h"

namespace scripting {
    extern lua::LuaState* state;
}

static int l_keycode(lua_State* L) {
    const char* name = lua_tostring(L, 1);
    lua_pushinteger(L, static_cast<int>(input_util::keycode_from(name)));
    return 1;
}

const luaL_Reg inputlib [] = {
    {"keycode", lua_wrap_errors<l_keycode>},
    {NULL, NULL}
};

