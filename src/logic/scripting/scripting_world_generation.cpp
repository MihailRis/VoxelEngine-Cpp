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

class LuaGeneratorScript : public GeneratorScript {
    scriptenv env;
    std::vector<Biome> biomes;
    uint biomeParameters;
    uint seaLevel;
public:
    LuaGeneratorScript(
        scriptenv env, 
        std::vector<Biome> biomes,
        uint biomeParameters,
        uint seaLevel)
    : env(std::move(env)), 
      biomes(std::move(biomes)), 
      biomeParameters(biomeParameters),
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

    std::vector<std::shared_ptr<Heightmap>> generateParameterMaps(
        const glm::ivec2& offset, const glm::ivec2& size, uint64_t seed
    ) override {
        std::vector<std::shared_ptr<Heightmap>> maps;

        auto L = lua::get_main_thread();
        lua::pushenv(L, *env);
        if (lua::getfield(L, "generate_biome_parameters")) {
            lua::pushivec_stack(L, offset);
            lua::pushivec_stack(L, size);
            lua::pushinteger(L, seed);
            if (lua::call_nothrow(L, 5, biomeParameters)) {
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

    void prepare(const Content* content) override {
        for (auto& biome : biomes) {
            for (auto& layer : biome.groundLayers.layers) {
                layer.rt.id = content->blocks.require(layer.block).rt.id;
            }
            for (auto& layer : biome.seaLayers.layers) {
                layer.rt.id = content->blocks.require(layer.block).rt.id;
            }
            for (auto& plant : biome.plants.plants) {
                plant.rt.id = content->blocks.require(plant.block).rt.id;
            }
        }
    }

    const std::vector<Biome>& getBiomes() const override {
        return biomes;
    }

    uint getBiomeParameters() const override {
        return biomeParameters;
    }

    uint getSeaLevel() const override {
        return seaLevel;
    }
};

static BlocksLayer load_layer(
    const dv::value& map, uint& lastLayersHeight, bool& hasResizeableLayer
) {
    const auto& name = map["block"].asString();
    int height = map["height"].asInteger();
    bool belowSeaLevel = true;
    map.at("below_sea_level").get(belowSeaLevel);

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
    const dv::value& layersArr, const std::string& fieldname
) {
    uint lastLayersHeight = 0;
    bool hasResizeableLayer = false;
    std::vector<BlocksLayer> layers;

    for (int i = 0; i < layersArr.size(); i++) {
        const auto& layerMap = layersArr[i];
        try {
            layers.push_back(
                load_layer(layerMap, lastLayersHeight, hasResizeableLayer));
        } catch (const std::runtime_error& err) {
            throw std::runtime_error(
                fieldname+" #"+std::to_string(i)+": "+err.what());
        }
    }
    return BlocksLayers {std::move(layers), lastLayersHeight};
}

static inline BiomePlants load_plants(
    const dv::value& biomeMap
) {
    float plantChance = 0.0f;
    biomeMap.at("plant_chance").get(plantChance);
    float plantsWeightSum = 0.0f;

    std::vector<PlantEntry> plants;
    if (biomeMap.has("plants")) {
        const auto& plantsArr = biomeMap["plants"];
        for (const auto& entry : plantsArr) {
            const auto& block = entry["block"].asString();
            float weight = entry["weight"].asNumber();
            if (weight <= 0.0f) {
                throw std::runtime_error("weight must be positive");
            }
            plantsWeightSum += weight;
            plants.push_back(PlantEntry {block, weight, {}});
        }
    }
    std::sort(plants.begin(), plants.end(), std::greater<PlantEntry>());
    return BiomePlants {
        std::move(plants), plantsWeightSum, plantChance};
}

static inline Biome load_biome(
    const dv::value& biomeMap,
    const std::string& name,
    uint parametersCount,
    int idx
) {
    std::vector<BiomeParameter> parameters;

    const auto& paramsArr = biomeMap["parameters"];
    if (paramsArr.size() < parametersCount) {
        throw std::runtime_error(
            std::to_string(parametersCount)+" parameters expected");
    }
    for (size_t i = 0; i < parametersCount; i++) {
        const auto& paramMap = paramsArr[i];
        float value = paramMap["value"].asNumber();
        float weight = paramMap["weight"].asNumber();
        parameters.push_back(BiomeParameter {value, weight});
    }

    BiomePlants plants = load_plants(biomeMap);
    BlocksLayers groundLayers = load_layers(biomeMap["layers"], "layers");
    BlocksLayers seaLayers = load_layers(biomeMap["sea_layers"], "sea_layers");
    return Biome {
        name,
        std::move(parameters),
        std::move(plants),
        std::move(groundLayers),
        std::move(seaLayers)};
}

std::unique_ptr<GeneratorScript> scripting::load_generator(
    const fs::path& file
) {
    auto env = create_environment();
    auto L = lua::get_main_thread();
    lua::stackguard _(L);

    lua::pop(L, load_script(*env, "generator", file));

    lua::pushenv(L, *env);
    auto root = lua::tovalue(L, -1);
    lua::pop(L);

    uint biomeParameters = root["biome_parameters"].asInteger();
    uint seaLevel = root["sea_level"].asInteger();

    std::vector<Biome> biomes;

    const auto& biomesMap = root["biomes"];
    for (const auto& [biomeName, biomeMap] : biomesMap.asObject()) {
        try {
            biomes.push_back(
                load_biome(biomeMap, biomeName, biomeParameters, -2));
        } catch (const std::runtime_error& err) {
            throw std::runtime_error("biome "+biomeName+": "+err.what());
        }
    }
    return std::make_unique<LuaGeneratorScript>(
        std::move(env), 
        std::move(biomes),
        biomeParameters,
        seaLevel);
}
