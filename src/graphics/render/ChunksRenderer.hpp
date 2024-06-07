#ifndef GRAPHICS_RENDER_CHUNKSRENDERER_HPP_
#define GRAPHICS_RENDER_CHUNKSRENDERER_HPP_

#include <queue>
#include <memory>
#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>

#include "../../voxels/Block.hpp"
#include "../../voxels/ChunksStorage.hpp"
#include "../../util/ThreadPool.hpp"

class Mesh;
class Chunk;
class Level;
class BlocksRenderer;
class ContentGfxCache;
struct EngineSettings;

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
        const EngineSettings* settings
    );
    virtual ~ChunksRenderer();

    std::shared_ptr<Mesh> render(const std::shared_ptr<Chunk>& chunk, bool important);
    void unload(const Chunk* chunk);

    std::shared_ptr<Mesh> getOrRender(const std::shared_ptr<Chunk>& chunk, bool important);
    std::shared_ptr<Mesh> get(Chunk* chunk);

    void update();
};

#endif // GRAPHICS_RENDER_CHUNKSRENDERER_HPP_
