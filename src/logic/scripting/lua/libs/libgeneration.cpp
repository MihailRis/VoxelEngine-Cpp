#include "api_lua.hpp"

#include "files/files.hpp"
#include "files/util.hpp"
#include "coders/binary_json.hpp"
#include "world/Level.hpp"
#include "world/generator/VoxelFragment.hpp"
#include "content/ContentLoader.hpp"
#include "engine.hpp"
#include "../lua_custom_types.hpp"

using namespace scripting;

static int l_save_structure(lua::State* L) {
    auto pointA = lua::tovec<3>(L, 1);
    auto pointB = lua::tovec<3>(L, 2);
    auto filename = lua::require_string(L, 3);
    if (!files::is_valid_name(filename)) {
        throw std::runtime_error("invalid file name");
    }
    bool saveEntities = lua::toboolean(L, 4);
    
    auto structure = VoxelFragment::create(level, pointA, pointB, saveEntities);
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

    auto structure = std::make_shared<VoxelFragment>();
    structure->deserialize(map);
    return lua::newuserdata<lua::LuaVoxelStructure>(L, std::move(structure));
}

/// @brief Get a list of all world generators
/// @return A table with the IDs of all world generators
static int l_get_generators(lua::State* L) {
    auto packs = engine->getAllContentPacks();

    lua::createtable(L, 0, 0);

    int i = 1;
    for (const auto& pack : packs) {
        auto pairs = ContentLoader::scanContent(pack, ContentType::GENERATOR);
        for (const auto& [name, caption] : pairs) {
            lua::pushstring(L, caption);
            lua::setfield(L, name);
            i++;
        }
    }
    return 1;
}

/// @brief Get the default world generator
/// @return The ID of the default world generator
static int l_get_default_generator(lua::State* L) {
    auto combined = engine->getResPaths()->readCombinedObject(
        EnginePaths::CONFIG_DEFAULTS.u8string()
    );
    return lua::pushstring(L, combined["generator"].asString());
}

const luaL_Reg generationlib[] = {
    {"save_structure", lua::wrap<l_save_structure>},
    {"load_structure", lua::wrap<l_load_structure>},
    {"get_generators", lua::wrap<l_get_generators>},
    {"get_default_generator", lua::wrap<l_get_default_generator>},
    {NULL, NULL}};
