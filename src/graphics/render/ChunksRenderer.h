#ifndef GRAPHICS_RENDER_CHUNKSRENDERER_H_
#define GRAPHICS_RENDER_CHUNKSRENDERER_H_

#include <queue>
#include <mutex>
#include <thread>
#include <memory>
#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>
#include <condition_variable>

#include "../../voxels/Block.h"
#include "../../voxels/ChunksStorage.h"
#include "../../settings.h"

class Mesh;
class Chunk;
class Level;
class BlocksRenderer;
class ContentGfxCache;

struct mesh_entry {
    BlocksRenderer& renderer;
    glm::ivec2 key;
};

struct Result {
    std::condition_variable& variable;
    int workerIndex;
    bool& locked;
    mesh_entry entry;
};

class ChunksRenderer {
    std::unique_ptr<BlocksRenderer> renderer;
    Level* level;
    std::unordered_map<glm::ivec2, std::shared_ptr<Mesh>> meshes;
    std::unordered_map<glm::ivec2, bool> inwork;
    std::vector<std::thread> threads;

    std::queue<Result> results;
    std::mutex resultsMutex;

    std::queue<std::shared_ptr<Chunk>> jobs;
    std::condition_variable jobsMutexCondition;
    std::mutex jobsMutex;

    bool working = true;
    const ContentGfxCache* cache;
    const EngineSettings& settings;
    std::vector<std::unique_lock<std::mutex>> workersBlocked;

    void threadLoop(int index);
    void process(std::shared_ptr<Chunk> chunk, BlocksRenderer& renderer);
public:
    ChunksRenderer(Level* level, 
                   const ContentGfxCache* cache, 
                   const EngineSettings& settings);
    virtual ~ChunksRenderer();

    std::shared_ptr<Mesh> render(std::shared_ptr<Chunk> chunk, bool important);
    void unload(Chunk* chunk);

    std::shared_ptr<Mesh> getOrRender(std::shared_ptr<Chunk> chunk, bool important);
    std::shared_ptr<Mesh> get(Chunk* chunk);

    void update();
};

#endif // GRAPHICS_RENDER_CHUNKSRENDERER_H_
