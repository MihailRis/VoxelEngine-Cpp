#pragma once

#include <vector>
#include <memory>
#include <algorithm>
#include <GL/glew.h>
#include <string>

#include <glm/glm.hpp>

#include "typedefs.hpp"

#include "presets/WeatherPreset.hpp"
#include "world/Weather.hpp"

class Level;
class Player;
class Camera;
class Batch3D;
class LineBatch;
class ChunksRenderer;
class ParticlesRenderer;
class BlockWrapsRenderer;
class PrecipitationRenderer;
class GuidesRenderer;
class TextsRenderer;
class Shader;
class Frustum;
class Engine;
class LevelFrontend;
class Skybox;
class PostProcessing;
class DrawContext;
class ModelBatch;
class Assets;
struct EngineSettings;

class WorldRenderer {
    Engine& engine;
    const Level& level;
    Player& player;
    const Assets& assets;
    std::unique_ptr<Frustum> frustumCulling;
    std::unique_ptr<LineBatch> lineBatch;
    std::unique_ptr<Batch3D> batch3d;
    std::unique_ptr<ModelBatch> modelBatch;
    std::unique_ptr<GuidesRenderer> guides;
    std::unique_ptr<ChunksRenderer> chunks;
    std::unique_ptr<Skybox> skybox;
    Weather weather {};
    
    float timer = 0.0f;
    bool debug = false;

    /// @brief Render block selection lines
    void renderBlockSelection();

    void renderHands(const Camera& camera, float delta);
    
    /// @brief Render lines (selection and debug)
    /// @param camera active camera
    /// @param linesShader shader used
    void renderLines(
        const Camera& camera, Shader& linesShader, const DrawContext& pctx
    );

    void renderBlockOverlay(const DrawContext& context);

    void setupWorldShader(
        Shader& shader,
        const Camera& camera,
        const EngineSettings& settings,
        float fogFactor
    );
public:
    std::unique_ptr<ParticlesRenderer> particles;
    std::unique_ptr<TextsRenderer> texts;
    std::unique_ptr<BlockWrapsRenderer> blockWraps;
    std::unique_ptr<PrecipitationRenderer> precipitation;

    static bool showChunkBorders;
    static bool showEntitiesDebug;

    WorldRenderer(Engine& engine, LevelFrontend& frontend, Player& player);
    ~WorldRenderer();

    void draw(
        const DrawContext& context, 
        Camera& camera, 
        bool hudVisible,
        bool pause,
        float delta,
        PostProcessing& postProcessing
    );

    /// @brief Render level without diegetic interface
    /// @param context graphics context
    /// @param camera active camera
    /// @param settings engine settings
    void renderLevel(
        const DrawContext& context, 
        const Camera& camera, 
        const EngineSettings& settings,
        float delta,
        bool pause,
        bool hudVisible
    );

    void clear();

    void setDebug(bool flag);

    Weather& getWeather();
};
