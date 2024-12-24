#include <cmath>
#include <filesystem>
#include <stdexcept>

#include "api_lua.hpp"
#include "assets/AssetsLoader.hpp"
#include "coders/compression.hpp"
#include "coders/gzip.hpp"
#include "coders/json.hpp"
#include "engine.hpp"
#include "files/engine_paths.hpp"
#include "files/files.hpp"
#include "lighting/Lighting.hpp"
#include "voxels/Chunk.hpp"
#include "voxels/Chunks.hpp"
#include "voxels/GlobalChunks.hpp"
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
    int x = (int)lua::tointeger(L, 1);
    int y = (int)lua::tointeger(L, 2);
    const auto& chunk = level->chunks->getChunk(x, y);
    if (chunk == nullptr) {
        lua::pushnil(L);
        return 0;
    }

    bool compress = false;
    if (lua::gettop(L) >= 3) {
        compress = lua::toboolean(L, 3);
    }
    std::vector<ubyte> chunk_data;
    if (compress) {
        size_t rle_compressed_size;
        size_t gzip_compressed_size;
        const auto& data_ptr = chunk->encode();
        ubyte* data = data_ptr.get();
        const auto& rle_compressed_data_ptr = compression::compress(
            data,
            CHUNK_DATA_LEN,
            rle_compressed_size,
            compression::Method::EXTRLE16
        );
        const auto& gzip_compressed_data = compression::compress(
            rle_compressed_data_ptr.get(),
            rle_compressed_size,
            gzip_compressed_size,
            compression::Method::GZIP
        );
        auto tmp = dataio::h2le(rle_compressed_size);
        chunk_data.reserve(gzip_compressed_size + sizeof(tmp));
        chunk_data.insert(
            chunk_data.begin() + 0, (char*)&tmp, ((char*)&tmp) + sizeof(tmp)
        );
        chunk_data.insert(
            chunk_data.begin() + sizeof(tmp),
            gzip_compressed_data.get(),
            gzip_compressed_data.get() + gzip_compressed_size
        );
    } else {
        const auto& data = chunk->encode();
        chunk_data.reserve(CHUNK_DATA_LEN);
        chunk_data.insert(
            chunk_data.begin(), data.get(), data.get() + CHUNK_DATA_LEN
        );
    }
    return lua::newuserdata<lua::LuaBytearray>(L, chunk_data);
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
    if (chunk == nullptr) {
        return 0;
    }
    if (is_compressed) {
        std::vector<ubyte>& raw_data = buffer->data();
        size_t gzip_decompressed_size =
            dataio::le2h(*(size_t*)(raw_data.data()));
        const auto& rle_data = compression::decompress(
            raw_data.data() + sizeof(gzip_decompressed_size),
            buffer->data().size() - sizeof(gzip_decompressed_size),
            gzip_decompressed_size,
            compression::Method::GZIP
        );
        const auto& data = compression::decompress(
            rle_data.get(),
            gzip_decompressed_size,
            CHUNK_DATA_LEN,
            compression::Method::EXTRLE16
        );
        chunk->decode(data.get());
    } else {
        chunk->decode(buffer->data().data());
    }

    auto chunksController = controller->getChunksController();
    if (chunksController == nullptr) {
        return 1;
    }

    Lighting& lighting = *chunksController->lighting;
    chunk->updateHeights();
    lighting.buildSkyLight(x, y);
    chunk->flags.modified = true;
    lighting.onChunkLoaded(x, y, true);

    chunk = level->chunks->getChunk(x - 1, y);
    if (chunk != nullptr) {
        chunk->flags.modified = true;
        lighting.onChunkLoaded(x - 1, y, true);
    }
    chunk = level->chunks->getChunk(x + 1, y);
    if (chunk != nullptr) {
        chunk->flags.modified = true;
        lighting.onChunkLoaded(x + 1, y, true);
    }
    chunk = level->chunks->getChunk(x, y - 1);
    if (chunk != nullptr) {
        chunk->flags.modified = true;
        lighting.onChunkLoaded(x, y - 1, true);
    }
    chunk = level->chunks->getChunk(x, y + 1);
    if (chunk != nullptr) {
        chunk->flags.modified = true;
        lighting.onChunkLoaded(x, y + 1, true);
    }

    return 1;
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
    {"count_chunks", lua::wrap<l_count_chunks>},
    {NULL, NULL}
};
