#include <cmath>
#include <filesystem>
#include <stdexcept>

#include "api_lua.hpp"
#include "assets/Assets.hpp"
#include "assets/AssetsLoader.hpp"
#include "coders/gzip.hpp"
#include "coders/json.hpp"
#include "engine.hpp"
#include "files/engine_paths.hpp"
#include "files/files.hpp"
#include "lighting/Lighting.hpp"
#include "voxels/Chunk.hpp"
#include "voxels/Chunks.hpp"
#include "world/Level.hpp"
#include "world/World.hpp"

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
    auto paths = engine->getPaths();
    auto worlds = paths->scanForWorlds();

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
    auto worldsDir = engine->getPaths()->getWorldFolderByName(name);
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
    int x = (int)lua::tointeger(L, 1);
    int y = (int)lua::tointeger(L, 2);
    const auto& chunk = level->chunks->getChunk(x, y);
    if (chunk->isEmpty()) {
        return 0;
    }

    bool compress = false;
    if (lua::gettop(L) >= 3) {
        compress = lua::toboolean(L, 3);
    }

    if (compress) {
        return lua::newuserdata<lua::LuaBytearray>(
            L, gzip::compress(chunk->encode().get(), CHUNK_DATA_LEN)
        );
    }
    const auto chunk_data = chunk->encode();
    const std::vector<ubyte> data(
        chunk_data.get(), chunk_data.get() + CHUNK_DATA_LEN
    );
    return lua::newuserdata<lua::LuaBytearray>(L, data);
}

static int l_set_chunk_data(lua::State* L) {
    int x = (int)lua::tointeger(L, 1);
    int y = (int)lua::tointeger(L, 2);
    auto buffer = lua::touserdata<lua::LuaBytearray>(L, 3);
    bool is_compressed = false;
    if (lua::gettop(L) >= 4) {
        is_compressed = lua::toboolean(L, 4);
    }
    auto chunk = level->chunks->getChunk(x, y);
    if (is_compressed) {
        auto data =
            gzip::decompress(buffer->data().data(), buffer->data().size());
        chunk->decode(data.data());
    } else {
        chunk->decode(buffer->data().data());
    }
    chunk->updateHeights();
    level->lighting->buildSkyLight(x, y);
    chunk->flags.modified = true;
    level->lighting->onChunkLoaded(x, y, true);

    chunk = level->chunks->getChunk(x - 1, y);
    if (chunk != nullptr) {
        chunk->flags.modified = true;
        level->lighting->onChunkLoaded(x - 1, y, true);
    }
    chunk = level->chunks->getChunk(x + 1, y);
    if (chunk != nullptr) {
        chunk->flags.modified = true;
        level->lighting->onChunkLoaded(x + 1, y, true);
    }
    chunk = level->chunks->getChunk(x, y - 1);
    if (chunk != nullptr) {
        chunk->flags.modified = true;
        level->lighting->onChunkLoaded(x, y - 1, true);
    }
    chunk = level->chunks->getChunk(x, y + 1);
    if (chunk != nullptr) {
        chunk->flags.modified = true;
        level->lighting->onChunkLoaded(x, y + 1, true);
    }

    return 1;
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
    {NULL, NULL}
};
