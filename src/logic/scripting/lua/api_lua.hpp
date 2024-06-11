#ifndef LOGIC_SCRIPTING_API_LUA_HPP_
#define LOGIC_SCRIPTING_API_LUA_HPP_

#include "lua_util.hpp"

#include <exception>
 
// Libraries
extern const luaL_Reg audiolib [];
extern const luaL_Reg blocklib [];
extern const luaL_Reg consolelib [];
extern const luaL_Reg corelib [];
extern const luaL_Reg filelib [];
extern const luaL_Reg guilib [];
extern const luaL_Reg hudlib [];
extern const luaL_Reg inputlib [];
extern const luaL_Reg inventorylib [];
extern const luaL_Reg itemlib [];
extern const luaL_Reg jsonlib [];
extern const luaL_Reg packlib [];
extern const luaL_Reg playerlib [];
extern const luaL_Reg timelib [];
extern const luaL_Reg tomllib [];
extern const luaL_Reg worldlib [];

// Lua Overrides
extern int l_print(lua_State* L);

#endif // LOGIC_SCRIPTING_API_LUA_HPP_
