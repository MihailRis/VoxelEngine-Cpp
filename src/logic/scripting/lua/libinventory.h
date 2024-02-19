#ifndef LOGIC_SCRIPTING_API_LIBINVENTORY_H_
#define LOGIC_SCRIPTING_API_LIBINVENTORY_H_

#include "lua_commons.h"

extern int l_inventory_get(lua_State* L);
extern int l_inventory_set(lua_State* L);
extern int l_inventory_size(lua_State* L);
extern int l_inventory_add(lua_State* L);
extern int l_inventory_get_block(lua_State* L);
extern int l_inventory_bind_block(lua_State* L);
extern int l_inventory_unbind_block(lua_State* L);

static const luaL_Reg inventorylib [] = {
    {"get", lua_wrap_errors<l_inventory_get>},
    {"set", lua_wrap_errors<l_inventory_set>},
    {"size", lua_wrap_errors<l_inventory_size>},
    {"add", lua_wrap_errors<l_inventory_add>},
    {"get_block", lua_wrap_errors<l_inventory_get_block>},
    {"bind_block", lua_wrap_errors<l_inventory_bind_block>},
    {"unbind_block", lua_wrap_errors<l_inventory_unbind_block>},
    {NULL, NULL}
};

#endif // LOGIC_SCRIPTING_API_LIBINVENTORY_H_
