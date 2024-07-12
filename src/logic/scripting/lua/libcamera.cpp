#include "api_lua.hpp"

#include "../../../content/Content.hpp"
#include "../../../world/Level.hpp"

using namespace scripting;

int l_index(lua::State* L) {
    auto name = lua::require_string(L, 1);
    auto& indices = content->getIndices(ResourceType::CAMERA);
    std::cout << "index: " << name << std::endl;
    return lua::pushinteger(L, indices.indexOf(name));
}

const luaL_Reg cameralib [] = {
    {"index", lua::wrap<l_index>},
    {NULL, NULL}
};
