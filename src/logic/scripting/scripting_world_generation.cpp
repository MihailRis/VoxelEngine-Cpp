#include "scripting.hpp"

#include "scripting_commons.hpp"
#include "typedefs.hpp"
#include "lua/lua_engine.hpp"
#include "lua/lua_custom_types.hpp"
#include "content/Content.hpp"
#include "voxels/Block.hpp"
#include "world/generator/GeneratorDef.hpp"

class LuaGeneratorScript : public GeneratorScript {
    scriptenv env;
    Biome biome;
    uint seaLevel;
public:
    LuaGeneratorScript(
        scriptenv env, 
        Biome biome,
        uint seaLevel)
    : env(std::move(env)), 
      biome(std::move(biome)), 
      seaLevel(seaLevel) 
      {}

    std::shared_ptr<Heightmap> generateHeightmap(
        const glm::ivec2& offset, const glm::ivec2& size, uint64_t seed
    ) override {
        auto L = lua::get_main_thread();
        lua::pushenv(L, *env);
        if (lua::getfield(L, "generate_heightmap")) {
            lua::pushivec_stack(L, offset);
            lua::pushivec_stack(L, size);
            lua::pushinteger(L, seed);
            if (lua::call_nothrow(L, 5)) {
                auto map = lua::touserdata<lua::LuaHeightmap>(L, -1)->getHeightmap();
                lua::pop(L, 2);
                return map;
            }
        }
        lua::pop(L);
        return std::make_shared<Heightmap>(size.x, size.y);
    }

    void prepare(const Content* content) override {
        for (auto& layer : biome.groundLayers.layers) {
            layer.rt.id = content->blocks.require(layer.block).rt.id;
        }
        for (auto& layer : biome.seaLayers.layers) {
            layer.rt.id = content->blocks.require(layer.block).rt.id;
        }
    }

    const Biome& getBiome() const override {
        return biome;
    }

    uint getSeaLevel() const override {
        return seaLevel;
    }
};

static BlocksLayer load_layer(
    lua::State* L, int idx, uint& lastLayersHeight, bool& hasResizeableLayer
) {
    lua::requirefield(L, "block");
    auto name = lua::require_string(L, -1);
    lua::pop(L);
    lua::requirefield(L, "height");
    int height = lua::tointeger(L, -1);
    lua::pop(L);
    bool belowSeaLevel = true;
    if (lua::getfield(L, "below_sea_level")) {
        belowSeaLevel = lua::toboolean(L, -1);
        lua::pop(L);
    }

    if (hasResizeableLayer) {
        lastLayersHeight += height;
    }
    if (height == -1) {
        if (hasResizeableLayer) {
            throw std::runtime_error("only one resizeable layer allowed");
        }
        hasResizeableLayer = true;
    }
    return BlocksLayer {name, height, belowSeaLevel, {}};
}

static inline BlocksLayers load_layers(
    lua::State* L, const std::string& fieldname
) {
    uint lastLayersHeight = 0;
    bool hasResizeableLayer = false;
    std::vector<BlocksLayer> layers;

    if (lua::getfield(L, fieldname)) {
        int len = lua::objlen(L, -1);
        for (int i = 1; i <= len; i++) {
            lua::rawgeti(L, i);
            try {
                layers.push_back(
                    load_layer(L, -1, lastLayersHeight, hasResizeableLayer));
            } catch (const std::runtime_error& err) {
                throw std::runtime_error(
                    fieldname+" #"+std::to_string(i)+": "+err.what());
            }
            lua::pop(L);
        }
        lua::pop(L);
    }
    return BlocksLayers {std::move(layers), lastLayersHeight};
}

std::unique_ptr<GeneratorScript> scripting::load_generator(
    const fs::path& file
) {
    auto env = create_environment();
    auto L = lua::get_main_thread();
    lua::stackguard _(L);

    lua::pop(L, load_script(*env, "generator", file));

    lua::pushenv(L, *env);

    uint seaLevel = 0;
    if (lua::getfield(L, "sea_level")) {
        seaLevel = lua::tointeger(L, -1);
        lua::pop(L);
    }

    uint lastGroundLayersHeight = 0;
    uint lastSeaLayersHeight = 0;
    bool hasResizeableGroundLayer = false;
    bool hasResizeableSeaLayer = false;

    BlocksLayers groundLayers;
    BlocksLayers seaLayers;
    try {
        groundLayers = load_layers(L, "layers");
        seaLayers = load_layers(L, "sea_layers");
    } catch (const std::runtime_error& err) {
        throw std::runtime_error(file.u8string()+": "+err.what());
    }
    lua::pop(L);
    return std::make_unique<LuaGeneratorScript>(
        std::move(env), 
        Biome {"default", std::move(groundLayers), std::move(seaLayers)}, 
        seaLevel);
}
