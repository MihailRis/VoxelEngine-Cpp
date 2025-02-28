#include "libhud.hpp"

using namespace scripting;

static int l_change(lua::State* L) {
    WeatherPreset weather {};
    weather.deserialize(lua::tovalue(L, 1));
    float time = lua::tonumber(L, 2);
    std::string name;
    if (lua::isstring(L, 3)) {
        name = lua::tostring(L, 3);
    }
    renderer->weather.change(std::move(weather), time, std::move(name));
    return 0;
}

static int l_get_current(lua::State* L) {
    if (renderer->weather.t > 0.5f) {
        return lua::pushstring(L, renderer->weather.nameB);
    } else {
        return lua::pushstring(L, renderer->weather.nameA);
    }
}

static int l_get_fall_intencity(lua::State* L) {
    const auto& a = renderer->weather.a;
    const auto& b = renderer->weather.b;
    float t = renderer->weather.t;
    return lua::pushnumber(L, 
        (a.fall.texture.empty() ? 0.0f : (1.0f - t)) +
        (b.fall.texture.empty() ? 0.0f : t)
    );
}

static int l_get_current_data(lua::State* L) {
    if (renderer->weather.t > 0.5f) {
        return lua::pushvalue(L, renderer->weather.b.serialize());
    } else {
        return lua::pushvalue(L, renderer->weather.a.serialize());
    }
}

const luaL_Reg weatherlib[] = {
    {"change", wrap_hud<l_change>},
    {"get_current", wrap_hud<l_get_current>},
    {"get_current_data", wrap_hud<l_get_current_data>},
    {"get_fall_intencity", wrap_hud<l_get_fall_intencity>},
    {NULL, NULL}
};
