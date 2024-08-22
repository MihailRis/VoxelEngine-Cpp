#include "api_lua.hpp"

#include "files/files.hpp"
#include "files/util.hpp"
#include "coders/binary_json.hpp"
#include "world/Level.hpp"
#include "world/generator/Structure.hpp"

using namespace scripting;

static int l_save_structure(lua::State* L) {
    auto pointA = lua::tovec<3>(L, 1);
    auto pointB = lua::tovec<3>(L, 2);
    auto filename = lua::require_string(L, 3);
    if (!files::is_valid_name(filename)) {
        throw std::runtime_error("invalid file name");
    }
    bool saveEntities = lua::toboolean(L, 4);
    
    auto structure = Structure::create(level, pointA, pointB, saveEntities);
    auto map = structure->serialize();

    auto bytes = json::to_binary(map.get());
    files::write_bytes(fs::u8path(filename), bytes.data(), bytes.size());
    return 0;
}

const luaL_Reg generationlib[] = {
    {"save_structure", lua::wrap<l_save_structure>},
    {NULL, NULL}};
