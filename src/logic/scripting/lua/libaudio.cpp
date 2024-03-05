#include "api_lua.h"
#include "lua_commons.h"

#include "../../../audio/audio.h"
#include "../../../engine.h"
#include "../scripting.h"

inline const char* DEFAULT_CHANNEL = "regular";

/// @brief audio.play_sound(
///            name: string, 
///            x: number, 
///            y: number, 
///            z: number, 
///            volume: number,
///            pitch: number,
///            channel: string = "regular"
///        )
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

/// @brief audio.stop(speakerid: integer) -> nil
static int l_audio_stop(lua_State* L) {
    lua::luaint id = lua_tonumber(L, 1);
    auto speaker = audio::get_speaker(id);
    if (speaker != nullptr) {
        speaker->stop();
    }
    return 0;
}

/// @brief audio.pause(speakerid: integer) -> nil 
static int l_audio_pause(lua_State* L) {
    lua::luaint id = lua_tonumber(L, 1);
    auto speaker = audio::get_speaker(id);
    if (speaker != nullptr) {
        speaker->pause();
    }
    return 0;
}

/// @brief audio.resume(speakerid: integer) -> nil 
static int l_audio_resume(lua_State* L) {
    lua::luaint id = lua_tonumber(L, 1);
    auto speaker = audio::get_speaker(id);
    if (speaker != nullptr && speaker->isPaused()) {
        speaker->play();
    }
    return 0;
}

/// @brief audio.set_volume(speakerid: integer, value: number) -> nil 
static int l_audio_set_volume(lua_State* L) {
    lua::luaint id = lua_tonumber(L, 1);
    auto speaker = audio::get_speaker(id);
    if (speaker != nullptr) {
        lua::luanumber value = lua_tonumber(L, 2);
        speaker->setVolume(static_cast<float>(value));
    }
    return 0;
}

/// @brief audio.set_pitch(speakerid: integer, value: number) -> nil 
static int l_audio_set_pitch(lua_State* L) {
    lua::luaint id = lua_tonumber(L, 1);
    auto speaker = audio::get_speaker(id);
    if (speaker != nullptr) {
        lua::luanumber value = lua_tonumber(L, 2);
        speaker->setPitch(static_cast<float>(value));
    }
    return 0;
}

/// @brief audio.set_position(speakerid: integer, x: number, y: number, z: number) -> nil
static int l_audio_set_position(lua_State* L) {
    lua::luaint id = lua_tonumber(L, 1);
    auto speaker = audio::get_speaker(id);
    if (speaker != nullptr) {
        lua::luanumber x = lua_tonumber(L, 2);
        lua::luanumber y = lua_tonumber(L, 3);
        lua::luanumber z = lua_tonumber(L, 4);
        speaker->setPosition(glm::vec3(
            static_cast<float>(x),
            static_cast<float>(y),
            static_cast<float>(z)
        ));
    }
    return 0;
}

/// @brief audio.set_velocity(speakerid: integer, x: number, y: number, z: number) -> nil
static int l_audio_set_velocity(lua_State* L) {
    lua::luaint id = lua_tonumber(L, 1);
    auto speaker = audio::get_speaker(id);
    if (speaker != nullptr) {
        lua::luanumber x = lua_tonumber(L, 2);
        lua::luanumber y = lua_tonumber(L, 3);
        lua::luanumber z = lua_tonumber(L, 4);
        speaker->setVelocity(glm::vec3(
            static_cast<float>(x),
            static_cast<float>(y),
            static_cast<float>(z)
        ));
    }
    return 0;
}

const luaL_Reg audiolib [] = {
    {"play_sound", lua_wrap_errors<l_audio_play_sound>},
    {"stop", lua_wrap_errors<l_audio_stop>},
    {"pause", lua_wrap_errors<l_audio_pause>},
    {"resume", lua_wrap_errors<l_audio_resume>},
    {"set_volume", lua_wrap_errors<l_audio_set_volume>},
    {"set_pitch", lua_wrap_errors<l_audio_set_pitch>},
    {"set_position", lua_wrap_errors<l_audio_set_position>},
    {"set_velocity", lua_wrap_errors<l_audio_set_velocity>},
    {NULL, NULL}
};
