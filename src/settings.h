#ifndef SRC_SETTINGS_H_
#define SRC_SETTINGS_H_

#include <string>

#include "constants.h"
#include "typedefs.h"
#include "data/observables.h"

struct AudioSettings {
    /// @brief try to initialize AL backend
    observable<bool> enabled = true;

    observable<float> volumeMaster = 1.0f;
    observable<float> volumeRegular = 1.0f;
    observable<float> volumeUI = 1.0f;
    observable<float> volumeAmbient = 1.0f;
    observable<float> volumeMusic = 1.0f;
};

struct DisplaySettings {
    /// @brief Is window in full screen mode
    observable<bool> fullscreen = false;
    /// @brief Window width (pixels)
    observable<int> width = 1280;
    /// @brief Window height (pixels)
    observable<int> height = 720;
    /// @brief Anti-aliasing samples
    observable<int> samples = 0;
    /// @brief GLFW swap interval value, 0 - unlimited fps, 1 - vsync
    observable<int> swapInterval = 1;
    /// @brief Window title */
    observable<std::string> title = "VoxelEngine-Cpp v" + 
        std::to_string(ENGINE_VERSION_MAJOR) + "." +
        std::to_string(ENGINE_VERSION_MINOR);
};

struct ChunksSettings {
    /// @brief Max milliseconds that engine uses for chunks loading only
    observable<uint> loadSpeed = 4;
    /// @brief Radius of chunks loading zone (chunk is unit) 
    observable<uint> loadDistance = 22;
    /// @brief Buffer zone where chunks are not unloading (chunk is unit)
    observable<uint> padding = 2;
};

struct CameraSettings {
    /// @brief Camera dynamic field of view effects
    observable<bool> fovEvents = true;
    /// @brief Camera movement shake
    observable<bool> shaking = true;
    /// @brief Camera field of view
    observable<float> fov = 90.0f;
    /// @brief Camera sensitivity
    observable<float> sensitivity = 2.0f;
};

struct GraphicsSettings {
    /// @brief Fog opacity is calculated as `pow(depth*k, fogCurve)` where k depends on chunksLoadDistance.
    /// Use values in range [1.0 - 2.0] where 1.0 is linear, 2.0 is quadratic
    observable<float> fogCurve = 1.6f;
    observable<float> gamma = 1.0f;
    /// @brief Enable blocks backlight to prevent complete darkness
    observable<bool> backlight = true;
    /// @brief Enable chunks frustum culling */
    observable<bool> frustumCulling = true;
    observable<int> skyboxResolution = 64 + 32;
};

struct DebugSettings {
    /// @brief Turns off chunks saving/loading
    observable<bool> generatorTestMode = false;
    observable<bool> showChunkBorders = false;
    observable<bool> doWriteLights = true;
};

struct UiSettings {
    observable<std::string> language = std::string("auto");
};

struct EngineSettings {
    AudioSettings audio;
    DisplaySettings display;
    ChunksSettings chunks;
    CameraSettings camera;
    GraphicsSettings graphics;
    DebugSettings debug;
    UiSettings ui;
};

#endif // SRC_SETTINGS_H_