# Audio

## Definitions

### Backend

Internal audio system implementation controlling audio output.
- NoAudio - dummy audio used on OpenAL initialize fail or if audio is disabled by the *settings.toml*: *\[audio\] enabled=false*
- ALAudio - OpenAL audio used

### Channel

Defines a sound sources category for group volume control, effects and pause.

Now the engine has following channels:
- *master* - controls other channels volume. Should not be used as a target channel when playing an audio.
- *ui* - ui elements sounds (button clicks and other)
- *regular* - world sounds, that will be paused with the game.
- *ambient* - same as *regular* but added for background and ambient sounds/streams like weather.
- *music* - music channel.

Channels are controlled by the engine and currently are not available in scripts.

### Speaker

One-time use playing audio controller for sound or stream. Speaker is destroying after stop (**stop** method call or audio track end if not looped)

> [!NOTE]
> Speaker access is performed using 64 bit integer identifiers that **will not be reused** after speaker destruction. You should avoid storing direct references and pointers instead of ID.

Speaker ID starts with 1, so 0 means audio play failure.

### Sound

Audio data loaded in memory to play multiple simultaneous instances from multiple sources. Can give access to loaded PCM data.

### PCMStream (PCM data source)

Stream used by an audio stream as an audio data source. Implementation depends on audio file format, not a backend. This interface may be used to implement network audio stream.

### Stream

Streaming audio. Not fully loading to the memory. Cannot be played via multiple speakers simultaneously.

## Formats support

- WAV: 8 and 16 bit supported (24 bit is not supported by OpenAL)
- OGG: implemented with libvorbis

## Additional information

> [!WARNING]
> **Stereo** audio played with OpenAL will ignore 3D position relative to the listener. Sounds that supposed to be played at specific world position must be **mono**

## Scripting Audio API

### Playing audio

Library **audio** contains available Audio API in Lua scripts.

```lua
audio.play_stream(
    -- audio file location (without entry point, but with extension included)
    name: string, 
    -- audio source world position
    x: number, y: number, z: number,
    -- audio gain (0.0 - 1.0)
    volume: number
    -- audio playback speed (positive number)
    pitch: number,
    -- [optional] channel name: regular/ambient/music/ui (default - regular)
    channel: string,
    -- [optional] loop stream (default - false)
    loop: bool
) -> int
```

Plays streaming audio from the specified file at the specified world position. Returns speaker ID.

```lua
audio.play_stream_2d(
    -- audio file location (without entry point, but with extension included)
    name: string, 
    -- audio gain (0.0 - 1.0)
    volume: number
    -- audio playback speed (positive number)
    pitch: number,
    -- [optional] channel name: regular/ambient/music/ui (default - regular)
    channel: string,
    -- [optional] loop stream (default - false)
    loop: bool
) -> int

```

Plays streaming audio from the specified file. Returns speaker ID.

```lua
audio.play_sound(
    -- name of a loaded sound without pack prefix, "sounds/", variant number and extension 
    -- example: "steps/stone" to play sound loaded from "sounds/steps/stone.ogg" or any of its variant
    -- variant will be randomly chosen
    name: string, 
    -- audio source world position
    x: number, y: number, z: number,
    -- audio gain (0.0 - 1.0)
    volume: number
    -- audio playback speed (positive number)
    pitch: number,
    -- [optional] channel name: regular/ambient/music/ui (default - regular)
    channel: string,
    -- [optional] loop sound (default - false)
    loop: bool
) -> int
```

Plays the specified sound on the specified position in world. Returns speaker ID.

```lua
audio.play_sound_2d(
    -- name of a loaded sound without pack prefix, "sounds/", variant number and extension 
    -- example: "steps/stone" to play sound loaded from "sounds/steps/stone.ogg" or any of its variant
    -- variant will be randomly chosen
    name: string, 
    -- audio gain (0.0 - 1.0)
    volume: number
    -- audio playback speed (positive number)
    pitch: number,
    -- [optional] channel name: regular/ambient/music/ui (default - regular)
    channel: string,
    -- [optional] loop sound (default - false)
    loop: bool
) -> int
```

Plays the specified sound. Returns speaker ID.

### Speaker interaction

Interaction with a non-existing or destroyed speaker will be ignored.


```lua
-- stop audio playback and destroy the speaker
audio.stop(speakerid: integer)

-- pause speaker
audio.pause(speakerid: integer)

-- unpause speaker
audio.resume(speakerid: integer)

-- set audio loop
audio.set_loop(speakerid: integer, state: bool)

-- check if audio is in loop (false if does not exists)
audio.is_loop(speakerid: integer) -> bool

-- get audio gain value (0.0 if does not exists)
audio.get_volume(speakerid: integer) -> number

-- set audio gain value
audio.set_volume(speakerid: integer, volume: number)

-- get audio playback speed (1.0 if does not exists)
audio.get_pitch(speakerid: integer) -> number

-- set audio playback speed
audio.set_pitch(speakerid: integer, pitch: number)

-- get current audio playback time in seconds (0.0 if does not exists)
audio.get_time(speakerid: integer) -> number

-- set audio playback time position in seconds
audio.set_time(speakerid: integer, time: number)

-- get audio source world position (nil if does not exists)
audio.get_position(speakerid: integer) -> number, number, number

-- set audio source world position
audio.set_position(speakerid: integer, x: number, y: number, z: number)

-- get audio source movement speed in world (nil if does not exists)
-- (OpenAL uses it for Doppler effect simulation)
audio.get_velocity(speakerid: integer) -> number, number, number

-- set audio source movement speed in world
-- (OpenAL uses it for Doppler effect simulation)
audio.set_velocity(speakerid: integer, x: number, y: number, z: number)

-- get audio duration
-- returns 0, if does not exists
-- also returns 0, if duration is unknown (example: radio)
audio.get_duration(speakerid: integer) -> number
```

### Other functions

```lua
-- get current number of alive speakers
audio.count_speakers() -> integer

-- get current number of playing streams
audio.count_streams() -> integer
```
