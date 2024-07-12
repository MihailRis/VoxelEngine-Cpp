#ifndef LOGIC_SCRIPTING_API_LUA_HPP_
#define LOGIC_SCRIPTING_API_LUA_HPP_

#include "lua_util.hpp"

#include <exception>

/// Definitions can be found in local .cpp files 
/// having same names as declarations

/// l_ prefix means that function is lua_CFunction:
///    int l_function_name(lua_State* L);
/// use following syntax:
///    int l_function_name(lua::State* L);

// Libraries
extern const luaL_Reg audiolib [];
extern const luaL_Reg blocklib [];
extern const luaL_Reg cameralib [];
extern const luaL_Reg consolelib [];
extern const luaL_Reg corelib [];
extern const luaL_Reg entitylib [];
extern const luaL_Reg filelib [];
extern const luaL_Reg guilib [];
extern const luaL_Reg hudlib [];
extern const luaL_Reg inputlib [];
extern const luaL_Reg inventorylib [];
extern const luaL_Reg itemlib [];
extern const luaL_Reg jsonlib [];
extern const luaL_Reg mat4lib [];
extern const luaL_Reg packlib [];
extern const luaL_Reg playerlib [];
extern const luaL_Reg timelib [];
extern const luaL_Reg tomllib [];
extern const luaL_Reg vec2lib []; // vecn.cpp
extern const luaL_Reg vec3lib []; // vecn.cpp
extern const luaL_Reg vec4lib []; // vecn.cpp
extern const luaL_Reg worldlib [];

// Components
extern const luaL_Reg skeletonlib [];
extern const luaL_Reg rigidbodylib [];
extern const luaL_Reg transformlib [];

// Lua Overrides
extern int l_print(lua::State* L);

#endif // LOGIC_SCRIPTING_API_LUA_HPP_
