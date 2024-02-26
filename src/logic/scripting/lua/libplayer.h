#ifndef LOGIC_SCRIPTING_LUA_LIBPLAYER_H_
#define LOGIC_SCRIPTING_LUA_LIBPLAYER_H_

#include "lua_commons.h"

/* == player library ==*/
extern int l_player_get_pos(lua_State* L);
extern int l_player_set_pos(lua_State* L);
extern int l_player_get_vel(lua_State* L);
extern int l_player_set_vel(lua_State* L);
extern int l_player_get_rot(lua_State* L);
extern int l_player_set_rot(lua_State* L);
extern int l_player_get_inv(lua_State* L);

static const luaL_Reg playerlib [] = {
    {"get_pos", lua_wrap_errors<l_player_get_pos>},
    {"set_pos", lua_wrap_errors<l_player_set_pos>},
    {"get_vel", lua_wrap_errors<l_player_get_vel>},
    {"set_vel", lua_wrap_errors<l_player_set_vel>},
    {"get_rot", lua_wrap_errors<l_player_get_rot>},
    {"set_rot", lua_wrap_errors<l_player_set_rot>},
    {"get_inventory", lua_wrap_errors<l_player_get_inv>},
    {NULL, NULL}
};

#endif // LOGIC_SCRIPTING_LUA_LIBPLAYER_H_
