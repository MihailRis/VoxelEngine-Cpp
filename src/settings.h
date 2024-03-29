#ifndef SRC_SETTINGS_H_
#define SRC_SETTINGS_H_

#include <string>

#include "data/setting.h"
#include "constants.h"
#include "typedefs.h"

struct AudioSettings {
    /// @brief try to initialize AL backend
    bool enabled = true;
    
    NumberSetting volumeMaster = NumberSetting::createPercent(1.0);
    NumberSetting volumeRegular = NumberSetting::createPercent(1.0);
    NumberSetting volumeUI = NumberSetting::createPercent(1.0);
    NumberSetting volumeAmbient = NumberSetting::createPercent(1.0);
    NumberSetting volumeMusic = NumberSetting::createPercent(1.0);
};

struct DisplaySettings {
    /// @brief Is window in full screen mode
    bool fullscreen = false;
    /// @brief Window width (pixels)
    int width = 1280;
    /// @brief Window height (pixels)
    int height = 720;
    /// @brief Anti-aliasing samples
    int samples = 0;
    /// @brief GLFW swap interval value, 0 - unlimited fps, 1 - vsync
    int swapInterval = 1;
    /// @brief Window title */
    std::string title = "VoxelEngine-Cpp v" + 
        std::to_string(ENGINE_VERSION_MAJOR) + "." +
        std::to_string(ENGINE_VERSION_MINOR);
};

struct ChunksSettings {
    /// @brief Max milliseconds that engine uses for chunks loading only
    uint loadSpeed = 4;
    /// @brief Radius of chunks loading zone (chunk is unit) 
    uint loadDistance = 22;
    /// @brief Buffer zone where chunks are not unloading (chunk is unit)
    uint padding = 2;
};

struct CameraSettings {
    /// @brief Camera dynamic field of view effects
    bool fovEvents = true;
    /// @brief Camera movement shake
    bool shaking = true;
    /// @brief Camera field of view
    float fov = 90.0f;
    /// @brief Camera sensitivity
    float sensitivity = 2.0f;
};

struct GraphicsSettings {
    /// @brief Fog opacity is calculated as `pow(depth*k, fogCurve)` where k depends on chunksLoadDistance.
    /// Use values in range [1.0 - 2.0] where 1.0 is linear, 2.0 is quadratic
    float fogCurve = 1.6f;
    float gamma = 1.0f;
    /// @brief Enable blocks backlight to prevent complete darkness
    bool backlight = true;
    /// @brief Enable chunks frustum culling */
    bool frustumCulling = true;
    int skyboxResolution = 64 + 32;
};

struct DebugSettings {
    /// @brief Turns off chunks saving/loading
    bool generatorTestMode = false;
    bool showChunkBorders = false;
    bool doWriteLights = true;
};

struct UiSettings {
    std::string language = "auto";
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
