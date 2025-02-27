#include "libhud.hpp"

using namespace scripting;

static int l_change(lua::State* L) {
    WeatherPreset weather {};
    weather.deserialize(lua::tovalue(L, 1));
    float time = lua::tonumber(L, 2);
    renderer->weather.change(std::move(weather), time);
    return 0;
}

const luaL_Reg weatherlib[] = {
    {"change", wrap_hud<l_change>},
    {NULL, NULL}
};
