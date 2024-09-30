#include "scripting.hpp"

#include <algorithm>
#include <functional>

#include "scripting_commons.hpp"
#include "typedefs.hpp"
#include "lua/lua_engine.hpp"
#include "lua/lua_custom_types.hpp"
#include "content/Content.hpp"
#include "voxels/Block.hpp"
#include "voxels/Chunk.hpp"
#include "data/dv.hpp"
#include "world/generator/GeneratorDef.hpp"
#include "util/timeutil.hpp"

class LuaGeneratorScript : public GeneratorScript {
    const GeneratorDef& def;
    scriptenv env;
public:
    LuaGeneratorScript(
        const GeneratorDef& def,
        scriptenv env)
    : def(def),
      env(std::move(env))
      {}

    std::shared_ptr<Heightmap> generateHeightmap(
        const glm::ivec2& offset, const glm::ivec2& size, uint64_t seed, uint bpd
    ) override {
        auto L = lua::get_main_thread();
        lua::pushenv(L, *env);
        if (lua::getfield(L, "generate_heightmap")) {
            lua::pushivec_stack(L, offset);
            lua::pushivec_stack(L, size);
            lua::pushinteger(L, seed);
            lua::pushinteger(L, bpd);
            if (lua::call_nothrow(L, 6)) {
                auto map = lua::touserdata<lua::LuaHeightmap>(L, -1)->getHeightmap();
                lua::pop(L, 2);
                return map;
            }
        }
        lua::pop(L);
        return std::make_shared<Heightmap>(size.x, size.y);
    }

    std::vector<std::shared_ptr<Heightmap>> generateParameterMaps(
        const glm::ivec2& offset, const glm::ivec2& size, uint64_t seed, uint bpd
    ) override {
        std::vector<std::shared_ptr<Heightmap>> maps;

        uint biomeParameters = def.biomeParameters;
        auto L = lua::get_main_thread();
        lua::pushenv(L, *env);
        if (lua::getfield(L, "generate_biome_parameters")) {
            lua::pushivec_stack(L, offset);
            lua::pushivec_stack(L, size);
            lua::pushinteger(L, seed);
            lua::pushinteger(L, bpd);
            if (lua::call_nothrow(L, 6, biomeParameters)) {
                for (int i = biomeParameters-1; i >= 0; i--) {
                    maps.push_back(
                        lua::touserdata<lua::LuaHeightmap>(L, -1-i)->getHeightmap());
                }
                lua::pop(L, 1+biomeParameters);
                return maps;
            }
        }
        lua::pop(L);
        for (uint i = 0; i < biomeParameters; i++) {
            maps.push_back(std::make_shared<Heightmap>(size.x, size.y));
        }
        return maps;
    }

    std::vector<StructurePlacement> placeStructures(
        const glm::ivec2& offset, const glm::ivec2& size, uint64_t seed,
        const std::shared_ptr<Heightmap>& heightmap, uint chunkHeight
    ) override {
        std::vector<StructurePlacement> placements;
        
        auto L = lua::get_main_thread();
        lua::stackguard _(L);
        lua::pushenv(L, *env);
        if (lua::getfield(L, "place_structures")) {
            lua::pushivec_stack(L, offset);
            lua::pushivec_stack(L, size);
            lua::pushinteger(L, seed);
            lua::newuserdata<lua::LuaHeightmap>(L, heightmap);
            lua::pushinteger(L, chunkHeight);
            if (lua::call_nothrow(L, 7, 1)) {
                int len = lua::objlen(L, -1);
                for (int i = 1; i <= len; i++) {
                    lua::rawgeti(L, i);

                    lua::rawgeti(L, 1);
                    int structIndex = 0;
                    if (lua::isstring(L, -1)) {
                        const auto& found = def.structuresIndices.find(lua::require_string(L, -1));
                        if (found != def.structuresIndices.end()) {
                            structIndex = found->second;
                        }
                    } else {
                        structIndex = lua::tointeger(L, -1);
                    }
                    lua::pop(L);

                    lua::rawgeti(L, 2);
                    glm::ivec3 pos = lua::tovec3(L, -1);
                    lua::pop(L);

                    lua::rawgeti(L, 3);
                    int rotation = lua::tointeger(L, -1) & 0b11;
                    lua::pop(L);

                    lua::pop(L);

                    placements.emplace_back(structIndex, pos, rotation);
                }
                lua::pop(L);
            }
        }
        return placements;
    }
};

std::unique_ptr<GeneratorScript> scripting::load_generator(
    const GeneratorDef& def, const fs::path& file, const std::string& dirPath
) {
    auto env = create_environment();
    auto L = lua::get_main_thread();
    lua::stackguard _(L);

    lua::pushenv(L, *env);
    lua::pushstring(L, dirPath);
    lua::setfield(L, "__DIR__");
    lua::pushstring(L, dirPath + "/script.lua");
    lua::setfield(L, "__FILE__");

    lua::pop(L);

    if (fs::exists(file)) {
        lua::pop(L, load_script(*env, "generator", file));
    } else {
        // Use default (empty) script
        lua::pop(L, lua::execute(lua::get_main_thread(), *env, "", "<empty>"));
    }

    return std::make_unique<LuaGeneratorScript>(
        def,
        std::move(env)
    );
}
