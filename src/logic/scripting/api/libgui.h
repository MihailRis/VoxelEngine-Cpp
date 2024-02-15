#ifndef LOGIC_SCRIPTING_API_LIBGUI_H_
#define LOGIC_SCRIPTING_API_LIBGUI_H_

#include <lua.hpp>

extern int l_gui_getviewport(lua_State* L);
extern int l_gui_getattr(lua_State* L);
extern int l_gui_setattr(lua_State* L);

static const luaL_Reg guilib [] = {
    {"get_viewport", l_gui_getviewport},
    {"getattr", l_gui_getattr},
    {"setattr", l_gui_setattr},
    {NULL, NULL}
};

#endif // LOGIC_SCRIPTING_API_LIBGUI_H_
