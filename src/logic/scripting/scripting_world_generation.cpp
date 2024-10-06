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
#include "files/files.hpp"
#include "debug/Logger.hpp"

using namespace lua;

static debug::Logger logger("generator-scripting");

class LuaGeneratorScript : public GeneratorScript {
    State* L;
    const GeneratorDef& def;
    scriptenv env;
public:
    LuaGeneratorScript(State* L, const GeneratorDef& def, scriptenv env)
        : L(L), def(def), env(std::move(env)) {
    }

    virtual ~LuaGeneratorScript() {
        env.reset();
        if (L != get_main_state()) {
            close(L);
        }
    }

    std::shared_ptr<Heightmap> generateHeightmap(
        const glm::ivec2& offset, const glm::ivec2& size, uint64_t seed, uint bpd
    ) override {
        pushenv(L, *env);
        if (getfield(L, "generate_heightmap")) {
            pushivec_stack(L, offset);
            pushivec_stack(L, size);
            pushinteger(L, seed);
            pushinteger(L, bpd);
            if (call_nothrow(L, 6)) {
                auto map = touserdata<LuaHeightmap>(L, -1)->getHeightmap();
                pop(L, 2);
                return map;
            }
        }
        pop(L);
        return std::make_shared<Heightmap>(size.x, size.y);
    }

    std::vector<std::shared_ptr<Heightmap>> generateParameterMaps(
        const glm::ivec2& offset, const glm::ivec2& size, uint64_t seed, uint bpd
    ) override {
        std::vector<std::shared_ptr<Heightmap>> maps;

        uint biomeParameters = def.biomeParameters;
        pushenv(L, *env);
        if (getfield(L, "generate_biome_parameters")) {
            pushivec_stack(L, offset);
            pushivec_stack(L, size);
            pushinteger(L, seed);
            pushinteger(L, bpd);
            if (call_nothrow(L, 6, biomeParameters)) {
                for (int i = biomeParameters-1; i >= 0; i--) {
                    maps.push_back(
                        touserdata<LuaHeightmap>(L, -1-i)->getHeightmap());
                }
                pop(L, 1+biomeParameters);
                return maps;
            }
        }
        pop(L);
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
        
        stackguard _(L);
        pushenv(L, *env);
        if (getfield(L, "place_structures")) {
            pushivec_stack(L, offset);
            pushivec_stack(L, size);
            pushinteger(L, seed);
            newuserdata<LuaHeightmap>(L, heightmap);
            pushinteger(L, chunkHeight);
            if (call_nothrow(L, 7, 1)) {
                int len = objlen(L, -1);
                for (int i = 1; i <= len; i++) {
                    rawgeti(L, i);

                    rawgeti(L, 1);
                    int structIndex = 0;
                    if (isstring(L, -1)) {
                        const auto& found = def.structuresIndices.find(
                            require_string(L, -1)
                        );
                        if (found != def.structuresIndices.end()) {
                            structIndex = found->second;
                        }
                    } else {
                        structIndex = tointeger(L, -1);
                    }
                    pop(L);

                    rawgeti(L, 2);
                    glm::ivec3 pos = tovec3(L, -1);
                    pop(L);

                    rawgeti(L, 3);
                    int rotation = tointeger(L, -1) & 0b11;
                    pop(L);

                    pop(L);

                    placements.emplace_back(structIndex, pos, rotation);
                }
                pop(L);
            }
        }
        return placements;
    }
};

std::unique_ptr<GeneratorScript> scripting::load_generator(
    const GeneratorDef& def, const fs::path& file, const std::string& dirPath
) {
    auto L = create_state(StateType::GENERATOR);
    auto env = create_environment(L);
    stackguard _(L);

    pushenv(L, *env);
    pushstring(L, dirPath);
    setfield(L, "__DIR__");
    pushstring(L, dirPath + "/script.lua");
    setfield(L, "__FILE__");

    pop(L);

    if (fs::exists(file)) {
        std::string src = files::read_string(file);
        logger.info() << "script (generator) " << file.u8string();
        pop(L, execute(L, *env, src, file.u8string()));
    } else {
        // Use default (empty) script
        pop(L, execute(L, *env, "", "<empty>"));
    }

    return std::make_unique<LuaGeneratorScript>(L, def, std::move(env));
}
