#ifndef WORLD_RENDERER_CPP
#define WORLD_RENDERER_CPP

#include <vector>
#include <memory>
#include <algorithm>
#include <GL/glew.h>
#include <string>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Level;
class Player;
class Camera;
class Batch3D;
class LineBatch;
class ChunksRenderer;
class Shader;
class Frustum;
class Engine;
class Chunks;
class LevelFrontend;
class Skybox;
class PostProcessing;
class GfxContext;
struct EngineSettings;

class WorldRenderer {
    Engine* engine;
    Level* level;
    Player* player;
    std::unique_ptr<PostProcessing> postProcessing;
    std::unique_ptr<Frustum> frustumCulling;
    std::unique_ptr<LineBatch> lineBatch;
    std::unique_ptr<ChunksRenderer> renderer;
    std::unique_ptr<Skybox> skybox;
    std::unique_ptr<Batch3D> batch3d;
    bool drawChunk(size_t index, Camera* camera, Shader* shader, bool culling);
    void drawChunks(Chunks* chunks, Camera* camera, Shader* shader);

    /// @brief Render level without diegetic interface
    /// @param context graphics context
    /// @param camera active camera
    /// @param settings engine settings
    void renderLevel(
        const GfxContext& context, 
        Camera* camera, 
        const EngineSettings& settings
    );

    /// @brief Render block selection lines
    /// @param camera active camera
    /// @param linesShader shader used
    void renderBlockSelection(Camera* camera, Shader* linesShader);

    /// @brief Render all debug lines (chunks borders, coord system guides)
    /// @param context graphics context
    /// @param camera active camera
    /// @param linesShader shader used
    /// @param settings engine settings
    void renderDebugLines(
        const GfxContext& context, 
        Camera* camera, 
        Shader* linesShader,
        const EngineSettings& settings
    );
public:
    WorldRenderer(Engine* engine, LevelFrontend* frontend, Player* player);
    ~WorldRenderer();

    void draw(const GfxContext& context, Camera* camera, bool hudVisible);
    void drawBorders(int sx, int sy, int sz, int ex, int ey, int ez);

    static float fog;
};


#endif // WORLD_RENDERER_CPP
