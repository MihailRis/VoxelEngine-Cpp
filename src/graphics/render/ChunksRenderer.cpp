#include "ChunksRenderer.hpp"
#include "BlocksRenderer.hpp"
#include "debug/Logger.hpp"
#include "graphics/core/Mesh.hpp"
#include "voxels/Chunk.hpp"
#include "world/Level.hpp"
#include "settings.hpp"

#include <iostream>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

static debug::Logger logger("chunks-render");

class RendererWorker : public util::Worker<Chunk, RendererResult> {
    Level* level;
    BlocksRenderer renderer;
public:
    RendererWorker(
        Level* level, 
        const ContentGfxCache* cache,
        const EngineSettings* settings
    ) : level(level), 
        renderer(settings->graphics.chunkMaxVertices.get(),
                 level->content, cache, settings)
    {}

    RendererResult operator()(const std::shared_ptr<Chunk>& chunk) override {
        renderer.build(chunk.get(), level->chunksStorage.get());
        if (renderer.isCancelled()) {
            return RendererResult {
                glm::ivec2(chunk->x, chunk->z), true, MeshData()};
        }
        auto meshData = renderer.createMesh();
        return RendererResult {
            glm::ivec2(chunk->x, chunk->z), false, std::move(meshData)};
    }
};

ChunksRenderer::ChunksRenderer(
    Level* level, 
    const ContentGfxCache* cache, 
    const EngineSettings* settings
) : level(level),
    threadPool(
        "chunks-render-pool",
        [=](){return std::make_shared<RendererWorker>(level, cache, settings);}, 
        [=](RendererResult& result){
            if (!result.cancelled) {
                meshes[result.key] = std::make_shared<Mesh>(result.meshData);
            }
            inwork.erase(result.key);
        }, settings->graphics.chunkMaxRenderers.get())
{
    threadPool.setStopOnFail(false);
    renderer = std::make_unique<BlocksRenderer>(
        settings->graphics.chunkMaxVertices.get(), 
        level->content, cache, settings
    );
    logger.info() << "created " << threadPool.getWorkersCount() << " workers";
}

ChunksRenderer::~ChunksRenderer() {
}

std::shared_ptr<Mesh> ChunksRenderer::render(const std::shared_ptr<Chunk>& chunk, bool important) {
    chunk->flags.modified = false;
    if (important) {
        auto mesh = renderer->render(chunk.get(), level->chunksStorage.get());
        meshes[glm::ivec2(chunk->x, chunk->z)] = mesh;
        return mesh;
    }
    glm::ivec2 key(chunk->x, chunk->z);
    if (inwork.find(key) != inwork.end()) {
        return nullptr;
    }
    inwork[key] = true;
    threadPool.enqueueJob(chunk);
    return nullptr;
}

void ChunksRenderer::unload(const Chunk* chunk) {
    auto found = meshes.find(glm::ivec2(chunk->x, chunk->z));
    if (found != meshes.end()) {
        meshes.erase(found);
    }
}

void ChunksRenderer::clear() {
    meshes.clear();
    inwork.clear();
    threadPool.clearQueue();
}

std::shared_ptr<Mesh> ChunksRenderer::getOrRender(const std::shared_ptr<Chunk>& chunk, bool important) {
    auto found = meshes.find(glm::ivec2(chunk->x, chunk->z));
    if (found == meshes.end()) {
        return render(chunk, important);
    }
    if (chunk->flags.modified) {
        render(chunk, important);
    }
    return found->second;
}

std::shared_ptr<Mesh> ChunksRenderer::get(Chunk* chunk) {
    auto found = meshes.find(glm::ivec2(chunk->x, chunk->z));
    if (found != meshes.end()) {
        return found->second;
    }
    return nullptr;
}

void ChunksRenderer::update() {
    threadPool.update();
}
