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
#include "util/stringutil.hpp"
#include "util/timeutil.hpp"

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

    std::vector<StructurePlacement> placeStructures(
        const GeneratorDef& def,
        const glm::ivec2& offset, const glm::ivec2& size, uint64_t seed,
        const std::shared_ptr<Heightmap>& heightmap
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
            if (lua::call_nothrow(L, 6, 1)) {
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

    void prepare(const GeneratorDef& def, const Content* content) override {
        for (auto& biome : biomes) {
            for (auto& layer : biome.groundLayers.layers) {
                layer.rt.id = content->blocks.require(layer.block).rt.id;
            }
            for (auto& layer : biome.seaLayers.layers) {
                layer.rt.id = content->blocks.require(layer.block).rt.id;
            }
            for (auto& plant : biome.plants.entries) {
                plant.rt.id = content->blocks.require(plant.name).rt.id;
            }
            for (auto& structure : biome.structures.entries) {
                const auto& found = def.structuresIndices.find(structure.name);
                if (found == def.structuresIndices.end()) {
                    throw std::runtime_error(
                        "no structure "+util::quote(structure.name)+" found");
                }
                structure.rt.id = found->second;
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

static inline BiomeElementList load_biome_element_list(
    const dv::value map,
    const std::string& chanceName,
    const std::string& arrName,
    const std::string& nameName
) {
    float chance = 0.0f;
    map.at(chanceName).get(chance);
    std::vector<WeightedEntry> entries;
    if (map.has(arrName)) {
        const auto& arr = map[arrName];
        for (const auto& entry : arr) {
            const auto& name = entry[nameName].asString();
            float weight = entry["weight"].asNumber();
            if (weight <= 0.0f) {
                throw std::runtime_error("weight must be positive");
            }
            entries.push_back(WeightedEntry {name, weight, {}});
        }
    }
    std::sort(entries.begin(), entries.end(), std::greater<WeightedEntry>());
    return BiomeElementList(std::move(entries), chance);
}

static inline BiomeElementList load_plants(const dv::value& biomeMap) {
    return load_biome_element_list(biomeMap, "plant_chance", "plants", "block");
}

static inline BiomeElementList load_structures(const dv::value map) {
    return load_biome_element_list(map, "structure_chance", "structures", "name");
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

    auto plants = load_plants(biomeMap);
    auto groundLayers = load_layers(biomeMap["layers"], "layers");
    auto seaLayers = load_layers(biomeMap["sea_layers"], "sea_layers");
    
    BiomeElementList structures;
    if (biomeMap.has("structures")) {
        structures = load_structures(biomeMap);
    }
    return Biome {
        name,
        std::move(parameters),
        std::move(plants),
        std::move(structures),
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
    uint seaLevel = 0;
    root.at("sea_level").get(seaLevel);

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
