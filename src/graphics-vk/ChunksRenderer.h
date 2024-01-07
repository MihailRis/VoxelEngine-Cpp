//
// Created by chelovek on 11/29/23.
//

#ifndef CHUNKRENDERER_H
#define CHUNKRENDERER_H

#include <memory>
#include <unordered_map>
#include <glm/glm.hpp>

#include "Vertices.h"
#include "../voxels/ChunksStorage.h"

class Chunk;
struct EngineSettings;
class ContentGfxCache;
class Level;
class BlocksRenderer;

namespace vulkan {

    template<typename T> class Mesh;

    class ChunksRenderer {
        BlocksRenderer *m_renderer;
        Level *m_level;
        std::unordered_map<glm::ivec2, std::shared_ptr<Mesh<Vertex3D>>> m_meshes;
    public:
        ChunksRenderer(Level *level,
                   const ContentGfxCache *cache,
                   const EngineSettings &settings);
        ~ChunksRenderer();

        std::shared_ptr<Mesh<Vertex3D>> render(Chunk *chunk);
        void unload(Chunk* chunk);

        std::shared_ptr<Mesh<Vertex3D>> getOrRender(Chunk *chunk);
        std::shared_ptr<Mesh<Vertex3D>> get(Chunk *chunk);
    };

} // vulkan

#endif //CHUNKRENDERER_H
