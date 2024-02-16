#ifndef LOGIC_SCRIPTING_API_LIBHUD_H_
#define LOGIC_SCRIPTING_API_LIBHUD_H_

#include <lua.hpp>

extern int l_hud_open_inventory(lua_State* L);
extern int l_hud_close_inventory(lua_State* L);
extern int l_hud_open_block(lua_State* L);

static const luaL_Reg hudlib [] = {
    {"open_inventory", l_hud_open_inventory},
    {"close_inventory", l_hud_close_inventory},
    {"open_block", l_hud_open_block},
    {NULL, NULL}
};

#endif // LOGIC_SCRIPTING_API_LIBHUD_H_
