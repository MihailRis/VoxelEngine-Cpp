#ifndef LOGIC_SCRIPTING_API_LUA_H_
#define LOGIC_SCRIPTING_API_LUA_H_

#include <exception>
#include "lua_commons.h"
 
// Libraries
extern const luaL_Reg packlib [];
extern const luaL_Reg timelib [];
extern const luaL_Reg filelib [];
extern const luaL_Reg worldlib [];
extern const luaL_Reg blocklib [];
extern const luaL_Reg itemlib [];
extern const luaL_Reg playerlib [];
extern const luaL_Reg inventorylib [];
extern const luaL_Reg guilib [];
extern const luaL_Reg hudlib [];
extern const luaL_Reg audiolib [];


// Lua Overrides
extern int l_print(lua_State* L);

#endif // LOGIC_SCRIPTING_API_LUA_H_
