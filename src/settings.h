#ifndef SRC_SETTINGS_H_
#define SRC_SETTINGS_H_

#include <string>
#include <unordered_map>

#include "data/setting.h"
#include "constants.h"
#include "typedefs.h"

struct AudioSettings {
    /// @brief try to initialize AL backend
    bool enabled = true;
    
    NumberSetting volumeMaster {1.0f, 0.0f, 1.0f, setting_format::percent};
    NumberSetting volumeRegular {1.0f, 0.0f, 1.0f, setting_format::percent};
    NumberSetting volumeUI {1.0f, 0.0f, 1.0f, setting_format::percent};
    NumberSetting volumeAmbient {1.0f, 0.0f, 1.0f, setting_format::percent};
    NumberSetting volumeMusic {1.0f, 0.0f, 1.0f, setting_format::percent};
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
    IntegerSetting loadSpeed = {4, 1, 32};
    /// @brief Radius of chunks loading zone (chunk is unit) 
    IntegerSetting loadDistance = {22, 3, 66};
    /// @brief Buffer zone where chunks are not unloading (chunk is unit)
    IntegerSetting padding = {2, 1, 8};
};

struct CameraSettings {
    /// @brief Camera dynamic field of view effects
    bool fovEvents = true;
    /// @brief Camera movement shake
    bool shaking = true;
    /// @brief Camera field of view
    NumberSetting fov {90.0f, 10, 120};
    /// @brief Camera sensitivity
    NumberSetting sensitivity {2.0f, 0.1f, 10.0f};
};

struct GraphicsSettings {
    /// @brief Fog opacity is calculated as `pow(depth*k, fogCurve)` where k depends on chunksLoadDistance.
    /// 1.0 is linear, 2.0 is quadratic
    NumberSetting fogCurve {1.6f, 1.0f, 6.0f};
    float gamma = 1.0f;
    /// @brief Enable blocks backlight to prevent complete darkness
    bool backlight = true;
    /// @brief Enable chunks frustum culling
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
