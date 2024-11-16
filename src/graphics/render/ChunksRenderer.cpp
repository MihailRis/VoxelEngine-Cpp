#include "ChunksRenderer.hpp"
#include "BlocksRenderer.hpp"
#include "debug/Logger.hpp"
#include "assets/Assets.hpp"
#include "graphics/core/Mesh.hpp"
#include "graphics/core/Shader.hpp"
#include "graphics/core/Texture.hpp"
#include "graphics/core/Atlas.hpp"
#include "voxels/Chunk.hpp"
#include "voxels/Chunks.hpp"
#include "world/Level.hpp"
#include "window/Camera.hpp"
#include "maths/FrustumCulling.hpp"
#include "util/listutil.hpp"
#include "settings.hpp"

#include <iostream>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "util/timeutil.hpp"

static debug::Logger logger("chunks-render");

size_t ChunksRenderer::visibleChunks = 0;

class RendererWorker : public util::Worker<std::shared_ptr<Chunk>, RendererResult> {
    const Level& level;
    BlocksRenderer renderer;
public:
    RendererWorker(
        const Level& level, 
        const ContentGfxCache& cache,
        const EngineSettings& settings
    ) : level(level), 
        renderer(settings.graphics.chunkMaxVertices.get(),
                 *level.content, cache, settings)
    {}

    RendererResult operator()(const std::shared_ptr<Chunk>& chunk) override {
        renderer.build(chunk.get(), level.chunks.get());
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
    const Level* level, 
    const Assets& assets,
    const Frustum& frustum,
    const ContentGfxCache& cache, 
    const EngineSettings& settings
) : level(*level),
    assets(assets),
    frustum(frustum),
    settings(settings),
    threadPool(
        "chunks-render-pool",
        [&](){return std::make_shared<RendererWorker>(*level, cache, settings);}, 
        [&](RendererResult& result){
            if (!result.cancelled) {
                auto meshData = std::move(result.meshData);
                meshes[result.key] = ChunkMesh {
                    std::make_shared<Mesh>(meshData.mesh),
                    std::move(meshData.sortingMesh)
                };
            }
            inwork.erase(result.key);
        }, settings.graphics.chunkMaxRenderers.get())
{
    threadPool.setStopOnFail(false);
    renderer = std::make_unique<BlocksRenderer>(
        settings.graphics.chunkMaxVertices.get(), 
        *level->content, cache, settings
    );
    logger.info() << "created " << threadPool.getWorkersCount() << " workers";

    const vattr attrs[]{ {3}, {2}, {1}, {0} };
    float buf[]{};
    sortedMesh = std::make_unique<Mesh>(buf, 0, attrs);
}

ChunksRenderer::~ChunksRenderer() {
}

std::shared_ptr<Mesh> ChunksRenderer::render(const std::shared_ptr<Chunk>& chunk, bool important) {
    chunk->flags.modified = false;
    if (important) {
        auto mesh = renderer->render(chunk.get(), level.chunks.get());
        meshes[glm::ivec2(chunk->x, chunk->z)] = ChunkMesh {
            std::move(mesh.mesh), std::move(mesh.sortingMesh)
        };
        return meshes[glm::ivec2(chunk->x, chunk->z)].mesh;
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
    return found->second.mesh;
}

void ChunksRenderer::update() {
    threadPool.update();
}

bool ChunksRenderer::drawChunk(
    size_t index, const Camera& camera, Shader& shader, bool culling
) {
    auto chunk = level.chunks->getChunks()[index];
    if (chunk == nullptr || !chunk->flags.lighted) {
        return false;
    }
    float distance = glm::distance(
        camera.position,
        glm::vec3(
            (chunk->x + 0.5f) * CHUNK_W,
            camera.position.y,
            (chunk->z + 0.5f) * CHUNK_D
        )
    );
    auto mesh = getOrRender(chunk, distance < CHUNK_W * 1.5f);
    if (mesh == nullptr) {
        return false;
    }
    if (culling) {
        glm::vec3 min(chunk->x * CHUNK_W, chunk->bottom, chunk->z * CHUNK_D);
        glm::vec3 max(
            chunk->x * CHUNK_W + CHUNK_W,
            chunk->top,
            chunk->z * CHUNK_D + CHUNK_D
        );

        if (!frustum.isBoxVisible(min, max)) return false;
    }
    glm::vec3 coord(chunk->x * CHUNK_W + 0.5f, 0.5f, chunk->z * CHUNK_D + 0.5f);
    glm::mat4 model = glm::translate(glm::mat4(1.0f), coord);
    shader.uniformMatrix("u_model", model);
    mesh->draw();
    return true;
}

void ChunksRenderer::drawChunks(
    const Camera& camera, Shader& shader
) {
    const auto& chunks = *level.chunks;
    const auto& atlas = assets.require<Atlas>("blocks");

    atlas.getTexture()->bind();
    update();

    // [warning] this whole method is not thread-safe for chunks

    int chunksWidth = chunks.getWidth();
    int chunksOffsetX = chunks.getOffsetX();
    int chunksOffsetY = chunks.getOffsetY();

    if (indices.size() != chunks.getVolume()) {
        indices.clear();
        for (int i = 0; i < chunks.getVolume(); i++) {
            indices.push_back(ChunksSortEntry {i, 0});
        }
    }
    float px = camera.position.x / static_cast<float>(CHUNK_W) - 0.5f;
    float pz = camera.position.z / static_cast<float>(CHUNK_D) - 0.5f;
    for (auto& index : indices) {
        float x = index.index % chunksWidth + chunksOffsetX - px;
        float z = index.index / chunksWidth + chunksOffsetY - pz;
        index.d = (x * x + z * z) * 1024;
    }
    util::insertion_sort(indices.begin(), indices.end());

    bool culling = settings.graphics.frustumCulling.get();

    visibleChunks = 0;
    //if (GLEW_ARB_multi_draw_indirect && false) {
        // TODO: implement Multi Draw Indirect chunks draw
    //} else {
        for (size_t i = 0; i < indices.size(); i++) {
            visibleChunks += drawChunk(indices[i].index, camera, shader, culling);
        }
    //}
}

void ChunksRenderer::drawSortedMeshes(const Camera& camera, Shader& shader) {
    timeutil::ScopeLogTimer log(444);
    
    const auto& atlas = assets.require<Atlas>("blocks");

    atlas.getTexture()->bind();

    std::vector<const SortingMeshEntry*> entries;

    const auto& chunks = level.chunks->getChunks();

    auto pposition = camera.position;
    
    size_t size = 0;
    bool culling = settings.graphics.frustumCulling.get();
    
    for (const auto& index : indices) {
        const auto& chunk = chunks[index.index];
        if (chunk == nullptr || !chunk->flags.lighted) {
            continue;
        }
        const auto& found = meshes.find(glm::ivec2(chunk->x, chunk->z));
        if (found == meshes.end()) {
            continue;
        }

        glm::vec3 min(chunk->x * CHUNK_W, chunk->bottom, chunk->z * CHUNK_D);
        glm::vec3 max(
            chunk->x * CHUNK_W + CHUNK_W,
            chunk->top,
            chunk->z * CHUNK_D + CHUNK_D
        );

        if (!frustum.isBoxVisible(min, max)) continue;

        auto& chunkEntries = found->second.sortingMesh.entries;
        
        for (auto& entry : chunkEntries) {
            entry.distance = static_cast<long long>(glm::distance2(entry.position, pposition));
        }
        std::sort(chunkEntries.begin(), chunkEntries.end());
        for (const auto& entry : chunkEntries) {
            size += entry.vertexData.size();
            entries.push_back(&entry);
        }
    }

    static util::Buffer<float> buffer;
    
    if (buffer.size() < size) {
        buffer = util::Buffer<float>(size);
    }
    size_t offset = 0;
    for (const auto& entry : entries) {
        std::memcpy(
            (buffer.data() + offset),
            entry->vertexData.data(),
            entry->vertexData.size() * sizeof(float)
        );
        offset += entry->vertexData.size();
    }
    sortedMesh->reload(buffer.data(), size / 6);

    shader.uniformMatrix("u_model", glm::mat4(1.0f));
    sortedMesh->draw();
}
