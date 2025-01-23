#include <cmath>
#include <filesystem>
#include <stdexcept>

#include "api_lua.hpp"
#include "assets/AssetsLoader.hpp"
#include "coders/json.hpp"
#include "engine/Engine.hpp"
#include "files/WorldFiles.hpp"
#include "files/engine_paths.hpp"
#include "files/files.hpp"
#include "lighting/Lighting.hpp"
#include "voxels/Chunk.hpp"
#include "voxels/Chunks.hpp"
#include "voxels/GlobalChunks.hpp"
#include "voxels/compressed_chunks.hpp"
#include "world/Level.hpp"
#include "world/World.hpp"
#include "logic/LevelController.hpp"
#include "logic/ChunksController.hpp"

using namespace scripting;
namespace fs = std::filesystem;

static WorldInfo& require_world_info() {
    if (level == nullptr) {
        throw std::runtime_error("no world open");
    }
    return level->getWorld()->getInfo();
}

static int l_is_open(lua::State* L) {
    return lua::pushboolean(L, level != nullptr);
}

static int l_get_list(lua::State* L) {
    const auto& paths = engine->getPaths();
    auto worlds = paths.scanForWorlds();

    lua::createtable(L, worlds.size(), 0);
    for (size_t i = 0; i < worlds.size(); i++) {
        lua::createtable(L, 0, 1);

        const auto& folder = worlds[i];

        auto root =
            json::parse(files::read_string(folder / fs::u8path("world.json")));
        const auto& versionMap = root["version"];
        int versionMajor = versionMap["major"].asInteger();
        int versionMinor = versionMap["minor"].asInteger();

        auto name = folder.filename().u8string();
        lua::pushstring(L, name);
        lua::setfield(L, "name");

        auto assets = engine->getAssets();
        std::string icon = "world#" + name + ".icon";
        if (!AssetsLoader::loadExternalTexture(
                assets,
                icon,
                {worlds[i] / fs::path("icon.png"),
                 worlds[i] / fs::path("preview.png")}
            )) {
            icon = "gui/no_world_icon";
        }
        lua::pushstring(L, icon);
        lua::setfield(L, "icon");

        lua::pushvec2(L, {versionMajor, versionMinor});
        lua::setfield(L, "version");

        lua::rawseti(L, i + 1);
    }
    return 1;
}

static int l_get_total_time(lua::State* L) {
    return lua::pushnumber(L, require_world_info().totalTime);
}

static int l_get_day_time(lua::State* L) {
    return lua::pushnumber(L, require_world_info().daytime);
}

static int l_set_day_time(lua::State* L) {
    auto value = lua::tonumber(L, 1);
    require_world_info().daytime = std::fmod(value, 1.0);
    return 0;
}

static int l_set_day_time_speed(lua::State* L) {
    auto value = lua::tonumber(L, 1);
    require_world_info().daytimeSpeed = std::abs(value);
    return 0;
}

static int l_get_day_time_speed(lua::State* L) {
    return lua::pushnumber(L, require_world_info().daytimeSpeed);
}

static int l_get_seed(lua::State* L) {
    return lua::pushinteger(L, require_world_info().seed);
}

static int l_exists(lua::State* L) {
    auto name = lua::require_string(L, 1);
    auto worldsDir = engine->getPaths().getWorldFolderByName(name);
    return lua::pushboolean(L, fs::is_directory(worldsDir));
}

static int l_is_day(lua::State* L) {
    auto daytime = require_world_info().daytime;
    return lua::pushboolean(L, daytime >= 0.333 && daytime <= 0.833);
}

static int l_is_night(lua::State* L) {
    auto daytime = require_world_info().daytime;
    return lua::pushboolean(L, daytime < 0.333 || daytime > 0.833);
}

static int l_get_generator(lua::State* L) {
    return lua::pushstring(L, require_world_info().generator);
}

static int l_get_chunk_data(lua::State* L) {
    int x = static_cast<int>(lua::tointeger(L, 1));
    int z = static_cast<int>(lua::tointeger(L, 2));
    const auto& chunk = level->chunks->getChunk(x, z);

    std::vector<ubyte> chunkData;
    if (chunk == nullptr) {
        auto& regions = level->getWorld()->wfile->getRegions();
        auto voxelData = regions.getVoxels(x, z);
        if (voxelData == nullptr) {
            return 0;
        }
        static util::Buffer<ubyte> rleBuffer(CHUNK_DATA_LEN * 2);
        auto metadata = regions.getBlocksData(x, z);
        chunkData =
            compressed_chunks::encode(voxelData.get(), metadata, rleBuffer);
    } else {
        chunkData = compressed_chunks::encode(*chunk);
    }
    return lua::newuserdata<lua::LuaBytearray>(L, std::move(chunkData));
}

static void integrate_chunk_client(Chunk& chunk) {
    int x = chunk.x;
    int z = chunk.z;
    auto chunksController = controller->getChunksController();
    
    Lighting& lighting = *chunksController->lighting;
    chunk.flags.loadedLights = false;
    chunk.flags.lighted = false;
    chunk.lightmap.clear();
    Lighting::prebuildSkyLight(chunk, *indices);

    for (int lz = -1; lz <= 1; lz++) {
        for (int lx = -1; lx <= 1; lx++) {
            if (std::abs(lx) + std::abs(lz) != 1) {
                continue;
            }
            if (auto other = level->chunks->getChunk(x + lx, z + lz)) {
                other->flags.modified = true;
            }
        }
    }
}

static int l_set_chunk_data(lua::State* L) {
    if (level == nullptr) {
        throw std::runtime_error("no open world");
    }

    int x = static_cast<int>(lua::tointeger(L, 1));
    int z = static_cast<int>(lua::tointeger(L, 2));
    auto buffer = lua::require_bytearray(L, 3);

    auto chunk = level->chunks->getChunk(x, z);
    if (chunk == nullptr) {
        return lua::pushboolean(L, false);
    }
    compressed_chunks::decode(
        *chunk, buffer.data(), buffer.size()
    );
    if (controller->getChunksController()->lighting == nullptr) {
        return lua::pushboolean(L, true);
    }
    integrate_chunk_client(*chunk);
    return lua::pushboolean(L, true);
}

static int l_save_chunk_data(lua::State* L) {
    if (level == nullptr) {
        throw std::runtime_error("no open world");
    }

    int x = static_cast<int>(lua::tointeger(L, 1));
    int z = static_cast<int>(lua::tointeger(L, 2));
    auto buffer = lua::require_bytearray(L, 3);

    compressed_chunks::save(
        x, z, std::move(buffer), level->getWorld()->wfile->getRegions()
    );
    return 0;
}

static int l_count_chunks(lua::State* L) {
    if (level == nullptr) {
        return 0;
    }
    return lua::pushinteger(L, level->chunks->size());
}

const luaL_Reg worldlib[] = {
    {"is_open", lua::wrap<l_is_open>},
    {"get_list", lua::wrap<l_get_list>},
    {"get_total_time", lua::wrap<l_get_total_time>},
    {"get_day_time", lua::wrap<l_get_day_time>},
    {"set_day_time", lua::wrap<l_set_day_time>},
    {"set_day_time_speed", lua::wrap<l_set_day_time_speed>},
    {"get_day_time_speed", lua::wrap<l_get_day_time_speed>},
    {"get_seed", lua::wrap<l_get_seed>},
    {"get_generator", lua::wrap<l_get_generator>},
    {"is_day", lua::wrap<l_is_day>},
    {"is_night", lua::wrap<l_is_night>},
    {"exists", lua::wrap<l_exists>},
    {"get_chunk_data", lua::wrap<l_get_chunk_data>},
    {"set_chunk_data", lua::wrap<l_set_chunk_data>},
    {"save_chunk_data", lua::wrap<l_save_chunk_data>},
    {"count_chunks", lua::wrap<l_count_chunks>},
    {NULL, NULL}
};
