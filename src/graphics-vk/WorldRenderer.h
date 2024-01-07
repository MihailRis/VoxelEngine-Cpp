//
// Created by chelovek on 11/29/23.
//

#ifndef WORLDRENDERER_H
#define WORLDRENDERER_H

#include <cstddef>
#include <vector>

#include "uniforms/DynamicConstants.h"

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
        std::vector<DynamicConstants> m_constantses;
        size_t m_constantIndex = 0;


        bool drawChunk(size_t index, Camera* camera, IShader* shader, bool culling);
        void drawChunks(Chunks* chunks, Camera* camera, IShader* shader);
    public:
        WorldRenderer(Engine* engine, Level* level, const ContentGfxCache* cache);
        ~WorldRenderer();

        void draw(const GfxContext& context, Camera* camera);
        void drawDebug(const GfxContext& context, Camera* camera);
        void drawBorders(int sx, int sy, int sz, int ex, int ey, int ez);

        static float fog;
    };

} // vulkan

#endif //WORLDRENDERER_H
