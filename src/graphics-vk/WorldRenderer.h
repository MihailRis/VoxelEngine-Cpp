//
// Created by chelovek on 11/29/23.
//

#ifndef WORLDRENDERER_H
#define WORLDRENDERER_H

#include <cstddef>

class GfxContext;
class ContentGfxCache;
class Frustum;
class Engine;
class Level;
class IShader;
class ITexture;
class Camera;
class Chunks;

namespace vulkan {
    class Skybox;
    class LineBatch;
    class ChunksRenderer;

    class WorldRenderer {
        Engine* m_engine;
        Level* m_level;
        Frustum* m_frustumCulling;
        LineBatch* m_lineBatch;
        ChunksRenderer* m_renderer;
        Skybox* m_skybox;

        bool drawChunk(size_t index, Camera* camera, IShader* shader, bool occlusion);
        void drawChunks(Chunks* chunks, Camera* camera, IShader* shader, bool occlusion);
    public:
        WorldRenderer(Engine* engine, Level* level, const ContentGfxCache* cache);
        ~WorldRenderer();

        void draw(const GfxContext& context, Camera* camera, bool occlusion);
        void drawDebug(const GfxContext& context, Camera* camera);
    };

} // vulkan

#endif //WORLDRENDERER_H
