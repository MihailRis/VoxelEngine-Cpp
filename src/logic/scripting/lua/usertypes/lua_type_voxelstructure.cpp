#include "../lua_custom_types.hpp"

#include "../lua_util.hpp"

#include "world/generator/VoxelFragment.hpp"
#include "util/stringutil.hpp"

using namespace lua;

LuaVoxelFragment::LuaVoxelFragment(std::shared_ptr<VoxelFragment> fragment)
    : fragment(std::move(fragment)) {}

LuaVoxelFragment::~LuaVoxelFragment() {
}

static int l_meta_tostring(lua::State* L) {
    return pushstring(L, "VoxelFragment(0x" + util::tohex(
        reinterpret_cast<uint64_t>(topointer(L, 1)))+")");
}

static int l_meta_index(lua::State* L) {
    auto fragment = touserdata<LuaVoxelFragment>(L, 1);
    if (fragment == nullptr) {
        return 0;
    }
    if (isstring(L, 2)) {
        auto fieldname = tostring(L, 2);
        if (!std::strcmp(fieldname, "size")) {
            return pushivec(L, fragment->getFragment()->getSize());
        }
    }
    return 0;
}


int LuaVoxelFragment::createMetatable(lua::State* L) {
    createtable(L, 0, 2);
    pushcfunction(L, lua::wrap<l_meta_tostring>);
    setfield(L, "__tostring");
    pushcfunction(L, lua::wrap<l_meta_index>);
    setfield(L, "__index");
    return 1;
}
