#include "ChunksRenderer.h"

#include "../../debug/Logger.h"
#include "../../graphics/core/Mesh.h"
#include "BlocksRenderer.h"
#include "../../voxels/Chunk.h"
#include "../../world/Level.h"

#include <iostream>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

static debug::Logger logger("chunks-render");

const uint RENDERER_CAPACITY = 9 * 6 * 6 * 3000;

class RendererWorker : public util::Worker<std::shared_ptr<Chunk>, RendererResult> {
    Level* level;
    BlocksRenderer renderer;
public:
    RendererWorker(
        Level* level, 
        const ContentGfxCache* cache, 
        const EngineSettings& settings
    ) : level(level), 
        renderer(RENDERER_CAPACITY, level->content, cache, settings)
    {}

    RendererResult operator()(const std::shared_ptr<Chunk>& chunk) override {
        renderer.build(chunk.get(), level->chunksStorage.get());
        return RendererResult {glm::ivec2(chunk->x, chunk->z), &renderer};
    }
};

ChunksRenderer::ChunksRenderer(
    Level* level, 
    const ContentGfxCache* cache, 
    const EngineSettings& settings
) : level(level),
    threadPool(
        "chunks-render-pool",
        [=](){return std::make_shared<RendererWorker>(level, cache, settings);}, 
        [=](RendererResult& mesh){
            meshes[mesh.key].reset(mesh.renderer->createMesh());
            inwork.erase(mesh.key);
        })
{
    renderer = std::make_unique<BlocksRenderer>(
        RENDERER_CAPACITY, level->content, cache, settings
    );
}

ChunksRenderer::~ChunksRenderer() {
}

std::shared_ptr<Mesh> ChunksRenderer::render(std::shared_ptr<Chunk> chunk, bool important) {
    chunk->setModified(false);

    if (important) {
        std::shared_ptr<Mesh> mesh (renderer->render(chunk.get(), level->chunksStorage.get()));
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

std::shared_ptr<Mesh> ChunksRenderer::getOrRender(std::shared_ptr<Chunk> chunk, bool important) {
    auto found = meshes.find(glm::ivec2(chunk->x, chunk->z));
    if (found == meshes.end()) {
        return render(chunk, important);
    }
    if (chunk->isModified()) {
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
