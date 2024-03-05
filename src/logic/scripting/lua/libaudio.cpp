#include "libaudio.h"

#include "../../../audio/audio.h"
#include "../../../engine.h"
#include "../scripting.h"

inline const char* DEFAULT_CHANNEL = "regular";

static int l_audio_play_sound(lua_State* L) {
    const char* name = lua_tostring(L, 1);
    lua::luanumber x = lua_tonumber(L, 2);
    lua::luanumber y = lua_tonumber(L, 3);
    lua::luanumber z = lua_tonumber(L, 4);
    lua::luanumber volume = lua_tonumber(L, 5);
    lua::luanumber pitch = lua_tonumber(L, 6);
    const char* channel = DEFAULT_CHANNEL;
    if (!lua_isnoneornil(L, 7)) {
        channel = lua_tostring(L, 7);
    }
    int channel_index = audio::get_channel_index(channel);
    if (channel_index == -1) {
        return 0;
    }
    
    auto assets = scripting::engine->getAssets();
    auto sound = assets->getSound(name);
    if (sound == nullptr) {
        return 0;
    }
    audio::speakerid_t id = audio::play(
        sound, glm::vec3(x, y, z), false, volume, pitch, false, audio::PRIORITY_NORMAL, channel_index
    );
    lua_pushinteger(L, static_cast<lua::luaint>(id));
    return 1;
}

const luaL_Reg audiolib [] = {
    {"play_sound", lua_wrap_errors<l_audio_play_sound>},
    {NULL, NULL}
};
