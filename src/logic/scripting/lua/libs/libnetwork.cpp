#include "api_lua.hpp"

#include "engine.hpp"
#include "network/Network.hpp"

using namespace scripting;

static int l_http_get(lua::State* L) {
    std::string url(lua::require_lstring(L, 1));

    lua::pushvalue(L, 2);
    auto onResponse = lua::create_lambda(L);

    engine->getNetwork().get(url, [onResponse](std::vector<char> bytes) {
        engine->postRunnable([=]() {
            onResponse({std::string(bytes.data(), bytes.size())});
        });
    });
    return 0;
}

static int l_http_get_binary(lua::State* L) {
    std::string url(lua::require_lstring(L, 1));

    lua::pushvalue(L, 2);
    auto onResponse = lua::create_lambda(L);

    engine->getNetwork().get(url, [onResponse](std::vector<char> bytes) {
        auto buffer = std::make_shared<util::Buffer<ubyte>>(
            reinterpret_cast<const ubyte*>(bytes.data()), bytes.size()
        );
        engine->postRunnable([=]() {
            onResponse({buffer});
        });
    });
    return 0;
}

const luaL_Reg networklib[] = {
    {"http_get", lua::wrap<l_http_get>},
    {"http_get_binary", lua::wrap<l_http_get_binary>},
    {NULL, NULL}
};
