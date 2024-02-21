#ifndef LOGIC_SCRIPTING_API_LIBGUI_H_
#define LOGIC_SCRIPTING_API_LIBGUI_H_

#include "lua_commons.h"

extern int l_gui_getviewport(lua_State* L);
extern int l_gui_getattr(lua_State* L);
extern int l_gui_setattr(lua_State* L);
extern int l_gui_get_env(lua_State* L);

static const luaL_Reg guilib [] = {
    {"get_viewport", lua_wrap_errors<l_gui_getviewport>},
    {"getattr", lua_wrap_errors<l_gui_getattr>},
    {"setattr", lua_wrap_errors<l_gui_setattr>},
    {"get_env", lua_wrap_errors<l_gui_get_env>},
    {NULL, NULL}
};

#endif // LOGIC_SCRIPTING_API_LIBGUI_H_
