#include "../lua_custom_types.hpp"

#include "../lua_util.hpp"

#include "world/generator/VoxelStructure.hpp"
#include "util/stringutil.hpp"

using namespace lua;

LuaVoxelStructure::LuaVoxelStructure(std::shared_ptr<VoxelStructure> structure)
    : structure(std::move(structure)) {}

LuaVoxelStructure::~LuaVoxelStructure() {
}

static int l_meta_tostring(lua::State* L) {
    return pushstring(L, "VoxelStructure(0x" + util::tohex(
        reinterpret_cast<uint64_t>(topointer(L, 1)))+")");
}

int LuaVoxelStructure::createMetatable(lua::State* L) {
    createtable(L, 0, 1);
    pushcfunction(L, lua::wrap<l_meta_tostring>);
    setfield(L, "__tostring");
    return 1;
}
