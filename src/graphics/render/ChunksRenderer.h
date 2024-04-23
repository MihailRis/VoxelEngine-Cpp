#ifndef GRAPHICS_RENDER_CHUNKSRENDERER_H_
#define GRAPHICS_RENDER_CHUNKSRENDERER_H_

#include <queue>
#include <memory>
#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>

#include "../../voxels/Block.h"
#include "../../voxels/ChunksStorage.h"
#include "../../util/ThreadPool.hpp"
#include "../../settings.h"

class Mesh;
class Chunk;
class Level;
class BlocksRenderer;
class ContentGfxCache;

struct RendererResult {
    glm::ivec2 key;
    BlocksRenderer* renderer;
};

class ChunksRenderer {
    Level* level;
    std::unique_ptr<BlocksRenderer> renderer;
    std::unordered_map<glm::ivec2, std::shared_ptr<Mesh>> meshes;
    std::unordered_map<glm::ivec2, bool> inwork;

    util::ThreadPool<Chunk, RendererResult> threadPool;
public:
    ChunksRenderer(
        Level* level, 
        const ContentGfxCache* cache, 
        const EngineSettings& settings
    );
    virtual ~ChunksRenderer();

    std::shared_ptr<Mesh> render(std::shared_ptr<Chunk> chunk, bool important);
    void unload(const Chunk* chunk);

    std::shared_ptr<Mesh> getOrRender(std::shared_ptr<Chunk> chunk, bool important);
    std::shared_ptr<Mesh> get(Chunk* chunk);

    void update();
};

#endif // GRAPHICS_RENDER_CHUNKSRENDERER_H_
