#pragma once

#include <queue>
#include <memory>
#include <vector>
#include <unordered_map>

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include "voxels/Block.hpp"
#include "util/ThreadPool.hpp"
#include "graphics/core/MeshData.hpp"
#include "commons.hpp"

class Mesh;
class Chunk;
class Level;
class Camera;
class Shader;
class Assets;
class Chunks;
class Frustum;
class BlocksRenderer;
class ContentGfxCache;
struct EngineSettings;

struct ChunksSortEntry {
    int index;
    int d;

    inline bool operator<(const ChunksSortEntry& o) const noexcept {
        return d > o.d;
    }
};

struct RendererResult {
    glm::ivec2 key;
    bool cancelled;
    ChunkMeshData meshData;
};

class ChunksRenderer {
    const Level& level;
    const Chunks& chunks;
    const Assets& assets;
    const Frustum& frustum;
    const EngineSettings& settings;

    std::unique_ptr<BlocksRenderer> renderer;
    std::unordered_map<glm::ivec2, ChunkMesh> meshes;
    std::unordered_map<glm::ivec2, bool> inwork;
    std::vector<ChunksSortEntry> indices;
    util::ThreadPool<std::shared_ptr<Chunk>, RendererResult> threadPool;
    const Mesh* retrieveChunk(
        size_t index, const Camera& camera, Shader& shader, bool culling
    );
public:
    ChunksRenderer(
        const Level* level,
        const Chunks& chunks,
        const Assets& assets,
        const Frustum& frustum,
        const ContentGfxCache& cache, 
        const EngineSettings& settings
    );
    virtual ~ChunksRenderer();

    const Mesh* render(
        const std::shared_ptr<Chunk>& chunk, bool important
    );
    void unload(const Chunk* chunk);
    void clear();

    const Mesh* getOrRender(
        const std::shared_ptr<Chunk>& chunk, bool important
    );
    void drawChunks(const Camera& camera, Shader& shader);

    void drawSortedMeshes(const Camera& camera, Shader& shader);

    void update();

    static size_t visibleChunks;
};
