#include "api_lua.h"
#include "lua_commons.h"
#include "lua_util.h"

#include "../../../audio/audio.h"
#include "../../../engine.h"
#include "../scripting.h"

inline const char* DEFAULT_CHANNEL = "regular";

inline int extract_channel_index(lua_State* L, int idx) {
    const char* channel = DEFAULT_CHANNEL;
    if (!lua_isnoneornil(L, idx)) {
        channel = lua_tostring(L, idx);
    }
    int index = audio::get_channel_index(channel);
    if (index == 0) {
        return -1;
    }
    return index;
}

inline audio::speakerid_t play_sound(
    const char* name,
    bool relative,
    lua::luanumber x,
    lua::luanumber y,
    lua::luanumber z,
    lua::luanumber volume,
    lua::luanumber pitch,
    bool loop,
    int channel
) {
    if (channel == -1)
        return 0;
    auto assets = scripting::engine->getAssets();
    auto sound = assets->getSound(name);
    if (sound == nullptr) {
        return 0;
    }

    return audio::play(
        sound, 
        glm::vec3(
            static_cast<float>(x), 
            static_cast<float>(y), 
            static_cast<float>(z)
        ), 
        relative, 
        volume, 
        pitch, 
        false, 
        audio::PRIORITY_NORMAL, 
        channel
    );
}

inline audio::speakerid_t play_stream(
    const char* filename,
    bool relative,
    lua::luanumber x,
    lua::luanumber y,
    lua::luanumber z,
    lua::luanumber volume,
    lua::luanumber pitch,
    bool loop,
    int channel
) {
    if (channel == -1)
        return 0;
    auto paths = scripting::engine->getResPaths();
    fs::path file = paths->find(filename);
    return audio::play_stream(
        file, 
        glm::vec3(
            static_cast<float>(x), 
            static_cast<float>(y), 
            static_cast<float>(z)
        ), 
        relative, 
        volume, 
        pitch, 
        loop, 
        channel
    );
}

/// @brief audio.play_stream(
///            name: string,
///            x: number, 
///            y: number, 
///            z: number, 
///            volume: number,
///            pitch: number,
///            channel: string = "regular",
///            loop: bool = false)
static int l_audio_play_stream(lua_State* L) {
    lua_pushinteger(L, static_cast<lua::luaint>(
        play_stream(
            lua_tostring(L, 1),
            false,
            lua_tonumber(L, 2),
            lua_tonumber(L, 3),
            lua_tonumber(L, 4),
            lua_tonumber(L, 5),
            lua_tonumber(L, 6),
            lua_toboolean(L, 8),
            extract_channel_index(L, 7)
        )
    ));
    return 1;
}

/// @brief audio.play_stream_2d(
///            name: string,
///            volume: number,
///            pitch: number,
///            channel: string = "regular",
///            loop: bool = false)
static int l_audio_play_stream_2d(lua_State* L) {
    lua_pushinteger(L, static_cast<lua::luaint>(
        play_stream(
            lua_tostring(L, 1),
            true,
            0.0, 0.0, 0.0,
            lua_tonumber(L, 2),
            lua_tonumber(L, 3),
            lua_toboolean(L, 5),
            extract_channel_index(L, 4)
        )
    ));
    return 1;
}

/// @brief audio.play_sound(
///            name: string, 
///            x: number, 
///            y: number, 
///            z: number, 
///            volume: number,
///            pitch: number,
///            channel: string = "regular",
///            loop: bool = false)
static int l_audio_play_sound(lua_State* L) {
    lua_pushinteger(L, static_cast<lua::luaint>(
        play_sound(
            lua_tostring(L, 1),
            false,
            lua_tonumber(L, 2),
            lua_tonumber(L, 3),
            lua_tonumber(L, 4),
            lua_tonumber(L, 5),
            lua_tonumber(L, 6),
            lua_toboolean(L, 8),
            extract_channel_index(L, 7)
        )
    ));
    return 1;
}

/// @brief audio.play_sound_2d(
///            name: string, 
///            volume: number,
///            pitch: number,
///            channel: string = "regular",
///            loop: bool = false)
static int l_audio_play_sound_2d(lua_State* L) {
    lua_pushinteger(L, static_cast<lua::luaint>(
        play_sound(
            lua_tostring(L, 1),
            true,
            0.0, 0.0, 0.0,
            lua_tonumber(L, 2),
            lua_tonumber(L, 3),
            lua_toboolean(L, 5),
            extract_channel_index(L, 4)
        )
    ));
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

/// @brief audio.set_loop(speakerid: integer, value: bool) -> nil 
static int l_audio_set_loop(lua_State* L) {
    lua::luaint id = lua_tonumber(L, 1);
    auto speaker = audio::get_speaker(id);
    if (speaker != nullptr) {
        bool value = lua_toboolean(L, 2);
        speaker->setLoop(value);
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

/// @brief audio.set_time(speakerid: integer, value: number) -> nil 
static int l_audio_set_time(lua_State* L) {
    lua::luaint id = lua_tonumber(L, 1);
    auto speaker = audio::get_speaker(id);
    if (speaker != nullptr) {
        lua::luanumber value = lua_tonumber(L, 2);
        speaker->setTime(static_cast<audio::duration_t>(value));
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

/// @brief audio.is_playing(speakerid: integer) -> bool 
static int l_audio_is_playing(lua_State* L) {
    lua::luaint id = lua_tonumber(L, 1);
    auto speaker = audio::get_speaker(id);
    if (speaker != nullptr) {
        lua_pushboolean(L, speaker->isPlaying());
        return 1;
    }
    lua_pushboolean(L, false);
    return 1;
}

/// @brief audio.is_paused(speakerid: integer) -> bool 
static int l_audio_is_paused(lua_State* L) {
    lua::luaint id = lua_tonumber(L, 1);
    auto speaker = audio::get_speaker(id);
    if (speaker != nullptr) {
        lua_pushboolean(L, speaker->isPaused());
        return 1;
    }
    lua_pushboolean(L, false);
    return 1;
}

/// @brief audio.is_loop(speakerid: integer) -> bool
static int l_audio_is_loop(lua_State* L) {
    lua::luaint id = lua_tonumber(L, 1);
    auto speaker = audio::get_speaker(id);
    if (speaker != nullptr) {
        lua_pushboolean(L, speaker->isLoop());
        return 1;
    }
    lua_pushboolean(L, false);
    return 1;
}

/// @brief audio.get_volume(speakerid: integer) -> number
static int l_audio_get_volume(lua_State* L) {
    lua::luaint id = lua_tonumber(L, 1);
    auto speaker = audio::get_speaker(id);
    if (speaker != nullptr) {
        lua_pushnumber(L, speaker->getVolume());
        return 1;
    }
    lua_pushnumber(L, 0.0);
    return 1;
}

/// @brief audio.get_pitch(speakerid: integer) -> number
static int l_audio_get_pitch(lua_State* L) {
    lua::luaint id = lua_tonumber(L, 1);
    auto speaker = audio::get_speaker(id);
    if (speaker != nullptr) {
        lua_pushnumber(L, speaker->getPitch());
        return 1;
    }
    lua_pushnumber(L, 1.0);
    return 1;
}

/// @brief audio.get_time(speakerid: integer) -> number
static int l_audio_get_time(lua_State* L) {
    lua::luaint id = lua_tonumber(L, 1);
    auto speaker = audio::get_speaker(id);
    if (speaker != nullptr) {
        lua_pushnumber(L, speaker->getTime());
        return 1;
    }
    lua_pushnumber(L, 0.0);
    return 1;
}

/// @brief audio.get_duration(speakerid: integer) -> number
static int l_audio_get_duration(lua_State* L) {
    lua::luaint id = lua_tonumber(L, 1);
    auto speaker = audio::get_speaker(id);
    if (speaker != nullptr) {
        lua_pushnumber(L, speaker->getDuration());
        return 1;
    }
    lua_pushnumber(L, 0.0);
    return 1;
}

/// @brief audio.get_position(speakerid: integer) -> number, number, number
static int l_audio_get_position(lua_State* L) {
    lua::luaint id = lua_tonumber(L, 1);
    auto speaker = audio::get_speaker(id);
    if (speaker != nullptr) {
        auto vec = speaker->getPosition();
        lua::pushvec3(L, vec);
        return 1;
    }
    return 0;
}

/// @brief audio.get_velocity(speakerid: integer) -> number, number, number
static int l_audio_get_velocity(lua_State* L) {
    lua::luaint id = lua_tonumber(L, 1);
    auto speaker = audio::get_speaker(id);
    if (speaker != nullptr) {
        auto vec = speaker->getVelocity();
        lua::pushvec3(L, vec);
        return 1;
    }
    return 0;
}

// @brief audio.count_speakers() -> integer
static int l_audio_count_speakers(lua_State* L) {
    lua_pushinteger(L, audio::count_speakers());
    return 1;
}

// @brief audio.count_streams() -> integer
static int l_audio_count_streams(lua_State* L) {
    lua_pushinteger(L, audio::count_streams());
    return 1;
}

const luaL_Reg audiolib [] = {
    {"play_sound", lua_wrap_errors<l_audio_play_sound>},
    {"play_sound_2d", lua_wrap_errors<l_audio_play_sound_2d>},
    {"play_stream", lua_wrap_errors<l_audio_play_stream>},
    {"play_stream_2d", lua_wrap_errors<l_audio_play_stream_2d>},
    {"stop", lua_wrap_errors<l_audio_stop>},
    {"pause", lua_wrap_errors<l_audio_pause>},
    {"resume", lua_wrap_errors<l_audio_resume>},
    {"set_loop", lua_wrap_errors<l_audio_set_loop>},
    {"set_volume", lua_wrap_errors<l_audio_set_volume>},
    {"set_pitch", lua_wrap_errors<l_audio_set_pitch>},
    {"set_time", lua_wrap_errors<l_audio_set_time>},
    {"set_position", lua_wrap_errors<l_audio_set_position>},
    {"set_velocity", lua_wrap_errors<l_audio_set_velocity>},
    {"is_playing", lua_wrap_errors<l_audio_is_playing>},
    {"is_paused", lua_wrap_errors<l_audio_is_paused>},
    {"is_loop", lua_wrap_errors<l_audio_is_loop>},
    {"get_volume", lua_wrap_errors<l_audio_get_volume>},
    {"get_pitch", lua_wrap_errors<l_audio_get_pitch>},
    {"get_time", lua_wrap_errors<l_audio_get_time>},
    {"get_duration", lua_wrap_errors<l_audio_get_duration>},
    {"get_position", lua_wrap_errors<l_audio_get_position>},
    {"get_velocity", lua_wrap_errors<l_audio_get_velocity>},
    {"count_speakers", lua_wrap_errors<l_audio_count_speakers>},
    {"count_streams", lua_wrap_errors<l_audio_count_streams>},
    {NULL, NULL}
};
