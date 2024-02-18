#ifndef LOGIC_SCRIPTING_API_LUA_H_
#define LOGIC_SCRIPTING_API_LUA_H_

#include <exception>
#include <lua.hpp>

template <lua_CFunction func> int lua_wrap_errors(lua_State *L) {
  int result = 0;
  try {
    result = func(L);
  }
  // transform exception with description into lua_error
  catch (std::exception &e) {
    luaL_error(L, e.what());
  }
  // Rethrow any other exception (lua error for example)
  catch (...) {
    throw;
  }

  return result;
}

/* == file library == */
extern int l_file_resolve(lua_State* L);
extern int l_file_read(lua_State* L);
extern int l_file_write(lua_State* L);
extern int l_file_exists(lua_State* L);
extern int l_file_isfile(lua_State* L);
extern int l_file_isdir(lua_State* L);
extern int l_file_length(lua_State* L);
extern int l_file_mkdir(lua_State* L);

static const luaL_Reg filelib [] = {
    {"resolve", lua_wrap_errors<l_file_resolve>},
    {"read", lua_wrap_errors<l_file_read>},
    {"write", lua_wrap_errors<l_file_write>},
    {"exists", lua_wrap_errors<l_file_exists>},
    {"isfile", lua_wrap_errors<l_file_isfile>},
    {"isdir", lua_wrap_errors<l_file_isdir>},
    {"length", lua_wrap_errors<l_file_length>},
    {"mkdir", lua_wrap_errors<l_file_mkdir>},
    {NULL, NULL}
};

/* == time library == */
extern int l_time_uptime(lua_State* L);
extern int l_time_delta(lua_State* L);

static const luaL_Reg timelib [] = {
    {"uptime", lua_wrap_errors<l_time_uptime>},
    {"delta", lua_wrap_errors<l_time_delta>},
    {NULL, NULL}
};
 
/* == pack library == */
extern int l_pack_get_folder(lua_State* L);

static const luaL_Reg packlib [] = {
    {"get_folder", lua_wrap_errors<l_pack_get_folder>},
    {NULL, NULL}
};

/* == world library == */
extern int l_world_get_total_time(lua_State* L);
extern int l_world_get_day_time(lua_State* L);
extern int l_world_set_day_time(lua_State* L);
extern int l_world_get_seed(lua_State* L);

static const luaL_Reg worldlib [] = {
    {"get_total_time", lua_wrap_errors<l_world_get_total_time>},
    {"get_day_time", lua_wrap_errors<l_world_get_day_time>},
    {"set_day_time", lua_wrap_errors<l_world_set_day_time>},
    {"get_seed", lua_wrap_errors<l_world_get_seed>},
    {NULL, NULL}
};

/* == player library ==*/
extern int l_player_get_pos(lua_State* L);
extern int l_player_get_rot(lua_State* L);
extern int l_player_set_rot(lua_State* L);
extern int l_player_set_pos(lua_State* L);
extern int l_player_get_inv(lua_State* L);

static const luaL_Reg playerlib [] = {
    {"get_pos", lua_wrap_errors<l_player_get_pos>},
    {"set_pos", lua_wrap_errors<l_player_set_pos>},
    {"get_rot", lua_wrap_errors<l_player_get_rot>},
    {"set_rot", lua_wrap_errors<l_player_set_rot>},
    {"get_inventory", lua_wrap_errors<l_player_get_inv>},
    {NULL, NULL}
};

/* == inventory library == */
extern int l_inventory_get(lua_State* L);
extern int l_inventory_set(lua_State* L);
extern int l_inventory_size(lua_State* L);
extern int l_inventory_add(lua_State* L);
extern int l_inventory_get_block(lua_State* L);

static const luaL_Reg inventorylib [] = {
    {"get", lua_wrap_errors<l_inventory_get>},
    {"set", lua_wrap_errors<l_inventory_set>},
    {"size", lua_wrap_errors<l_inventory_size>},
    {"add", lua_wrap_errors<l_inventory_add>},
    {"get_block", lua_wrap_errors<l_inventory_get_block>},
    {NULL, NULL}
};

/* == item library == */
extern int l_item_name(lua_State* L);
extern int l_item_index(lua_State* L);
extern int l_item_stack_size(lua_State* L);
extern int l_item_defs_count(lua_State* L);

static const luaL_Reg itemlib [] = {
    {"index", lua_wrap_errors<l_item_index>},
    {"name", lua_wrap_errors<l_item_name>},
    {"stack_size", lua_wrap_errors<l_item_stack_size>},
    {"defs_count", lua_wrap_errors<l_item_defs_count>},
    {NULL, NULL}
};

/* == blocks-related functions == */
extern int l_block_name(lua_State* L);
extern int l_is_solid_at(lua_State* L);
extern int l_blocks_count(lua_State* L);
extern int l_block_index(lua_State* L);
extern int l_set_block(lua_State* L);
extern int l_get_block(lua_State* L);
extern int l_get_block_x(lua_State* L);
extern int l_get_block_y(lua_State* L);
extern int l_get_block_z(lua_State* L);
extern int l_get_block_states(lua_State* L);
extern int l_set_block_states(lua_State* L);
extern int l_get_block_user_bits(lua_State* L);
extern int l_set_block_user_bits(lua_State* L);
extern int l_is_replaceable_at(lua_State* L);

// Modified version of luaB_print from lbaselib.c
extern int l_print(lua_State* L);

#endif // LOGIC_SCRIPTING_API_LUA_H_
