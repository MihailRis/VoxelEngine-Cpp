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

ChunksRenderer::ChunksRenderer(Level* level, const ContentGfxCache* cache, const EngineSettings& settings) 
: level(level), cache(cache), settings(settings) {
    const int MAX_FULL_CUBES = 3000;
    renderer = std::make_unique<BlocksRenderer>(
        9 * 6 * 6 * MAX_FULL_CUBES, level->content, cache, settings
    );

    const uint num_threads = std::thread::hardware_concurrency();
    for (uint i = 0; i < num_threads; i++) {
        threads.emplace_back(&ChunksRenderer::threadLoop, this, i);
        workersBlocked.emplace_back();
    }
    logger.info() << "created " << num_threads << " rendering threads";
}

ChunksRenderer::~ChunksRenderer() {
    {
        std::unique_lock<std::mutex> lock(jobsMutex);
        working = false;
    }

    resultsMutex.lock();
    while (!results.empty()) {
        Result entry = results.front();
        results.pop();
        entry.locked = false;
        entry.variable.notify_all();
    }
    resultsMutex.unlock();

    jobsMutexCondition.notify_all();
    for (auto& thread : threads) {
        thread.join();
    }
}

void ChunksRenderer::threadLoop(int index) {
    const int MAX_FULL_CUBES = 3000;
    BlocksRenderer renderer(
        9 * 6 * 6 * MAX_FULL_CUBES, level->content, cache, settings
    );

    std::condition_variable variable;
    std::mutex mutex;
    bool locked = false;
    while (working) {
        std::shared_ptr<Chunk> chunk;
        {
            std::unique_lock<std::mutex> lock(jobsMutex);
            jobsMutexCondition.wait(lock, [this] {
                return !jobs.empty() || !working;
            });
            if (!working) {
                break;
            }
            chunk = jobs.front();
            jobs.pop();
        }
        process(chunk, renderer);
        {
            resultsMutex.lock();
            results.push(Result {variable, index, locked, {renderer, glm::ivec2(chunk->x, chunk->z)}});
            locked = true;
            resultsMutex.unlock();
        }
        {
            std::unique_lock<std::mutex> lock(mutex);
            variable.wait(lock, [&] {
                return !working || !locked;
            });
        }
    }
}

void ChunksRenderer::process(std::shared_ptr<Chunk> chunk, BlocksRenderer& renderer) {
    renderer.build(chunk.get(), level->chunksStorage.get());
}

std::shared_ptr<Mesh> ChunksRenderer::render(std::shared_ptr<Chunk> chunk, bool important) {
    chunk->setModified(false);

    if (important) {
        Mesh* mesh = renderer->render(chunk.get(), level->chunksStorage.get());
        auto sptr = std::shared_ptr<Mesh>(mesh);
        meshes[glm::ivec2(chunk->x, chunk->z)] = sptr;
        return sptr;
    }

    glm::ivec2 key(chunk->x, chunk->z);
    if (inwork.find(key) != inwork.end()) {
        return nullptr;
    }

    inwork[key] = true;
    jobsMutex.lock();
    jobs.push(chunk);
    jobsMutex.unlock();
    jobsMutexCondition.notify_one();
    return nullptr;
}

void ChunksRenderer::unload(Chunk* chunk) {
    auto found = meshes.find(glm::ivec2(chunk->x, chunk->z));
    if (found != meshes.end()) {
        meshes.erase(found);
    }
}

std::shared_ptr<Mesh> ChunksRenderer::getOrRender(std::shared_ptr<Chunk> chunk, bool important) {
    auto found = meshes.find(glm::ivec2(chunk->x, chunk->z));
    if (found != meshes.end()){
        if (chunk->isModified()) {
            render(chunk, important);
        }
        return found->second;
    }
    return render(chunk, important);
}

std::shared_ptr<Mesh> ChunksRenderer::get(Chunk* chunk) {
    auto found = meshes.find(glm::ivec2(chunk->x, chunk->z));
    if (found != meshes.end()) {
        return found->second;
    }
    return nullptr;
}

void ChunksRenderer::update() {
    resultsMutex.lock();
    while (!results.empty()) {
        Result entry = results.front();
        mesh_entry mesh = entry.entry;
        results.pop();
        meshes[mesh.key] = std::shared_ptr<Mesh>(mesh.renderer.createMesh());
        inwork.erase(mesh.key);
        entry.locked = false;
        entry.variable.notify_all();
    }
    resultsMutex.unlock();
}
