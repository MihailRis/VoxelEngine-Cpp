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

static int l_save_fragment(lua::State* L) {
    const auto& paths = engine->getPaths();
    auto fragment = lua::touserdata<lua::LuaVoxelFragment>(L, 1);
    auto file = paths.resolve(lua::require_string(L, 2), true);
    auto map = fragment->getFragment()->serialize();
    auto bytes = json::to_binary(map, true);
    files::write_bytes(file, bytes.data(), bytes.size());
    return 0;
}

static int l_create_fragment(lua::State* L) {
    auto pointA = lua::tovec<3>(L, 1);
    auto pointB = lua::tovec<3>(L, 2);
    bool crop = lua::toboolean(L, 3);
    bool saveEntities = lua::toboolean(L, 4);

    auto fragment =
        VoxelFragment::create(*level, pointA, pointB, crop, saveEntities);
    return lua::newuserdata<lua::LuaVoxelFragment>(
        L, std::shared_ptr<VoxelFragment>(std::move(fragment))
    );
}

static int l_load_fragment(lua::State* L) {
    const auto& paths = engine->getPaths();
    auto filename = lua::require_string(L, 1);
    auto path = paths.resolve(filename);
    if (!std::filesystem::exists(path)) {
        throw std::runtime_error("file "+path.u8string()+" does not exist");
    }
    auto map = files::read_binary_json(path);

    auto fragment = std::make_shared<VoxelFragment>();
    fragment->deserialize(map);
    fragment->prepare(*content);
    return lua::newuserdata<lua::LuaVoxelFragment>(L, std::move(fragment));
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
    {"create_fragment", lua::wrap<l_create_fragment>},
    {"save_fragment", lua::wrap<l_save_fragment>},
    {"load_fragment", lua::wrap<l_load_fragment>},
    {"get_generators", lua::wrap<l_get_generators>},
    {"get_default_generator", lua::wrap<l_get_default_generator>},
    {NULL, NULL}};
