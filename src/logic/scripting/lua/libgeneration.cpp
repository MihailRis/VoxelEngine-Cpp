#include "api_lua.hpp"

#include "files/files.hpp"
#include "files/util.hpp"
#include "coders/binary_json.hpp"
#include "world/Level.hpp"
#include "world/generator/VoxelStructure.hpp"
#include "engine.hpp"
#include "lua_custom_types.hpp"

using namespace scripting;

static int l_save_structure(lua::State* L) {
    auto pointA = lua::tovec<3>(L, 1);
    auto pointB = lua::tovec<3>(L, 2);
    auto filename = lua::require_string(L, 3);
    if (!files::is_valid_name(filename)) {
        throw std::runtime_error("invalid file name");
    }
    bool saveEntities = lua::toboolean(L, 4);
    
    auto structure = VoxelStructure::create(level, pointA, pointB, saveEntities);
    auto map = structure->serialize();

    auto bytes = json::to_binary(map);
    files::write_bytes(fs::u8path(filename), bytes.data(), bytes.size());
    return 0;
}

static int l_load_structure(lua::State* L) {
    auto paths = engine->getPaths();
    auto [prefix, filename] = EnginePaths::parsePath(lua::require_string(L, 1));

    auto path = paths->resolve(prefix+":generators/"+filename+".vox");
    if (!std::filesystem::exists(path)) {
        throw std::runtime_error("file "+path.u8string()+" does not exist");
    }
    auto map = files::read_binary_json(path);

    auto structure = std::make_shared<VoxelStructure>();
    structure->deserialize(map);
    return lua::newuserdata<lua::LuaVoxelStructure>(L, std::move(structure));
}

const luaL_Reg generationlib[] = {
    {"save_structure", lua::wrap<l_save_structure>},
    {"load_structure", lua::wrap<l_load_structure>},
    {NULL, NULL}};
