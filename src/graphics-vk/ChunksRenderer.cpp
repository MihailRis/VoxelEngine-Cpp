//
// Created by chelovek on 11/29/23.
//

#include "ChunksRenderer.h"

#include "../world/Level.h"
#include "../voxels/Chunk.h"
#include "../graphics/BlocksRenderer.h"
#include "Mesh.h"

namespace vulkan {
    ChunksRenderer::ChunksRenderer(Level* level, const ContentGfxCache* cache, const EngineSettings& settings)
        : m_level(level) {
        constexpr int MAX_FULL_CUBES = 3000;
        m_renderer = new BlocksRenderer(9 * 6 * 6 * MAX_FULL_CUBES, level->content, cache, settings);
    }

    ChunksRenderer::~ChunksRenderer() {
        delete m_renderer;
    }

    std::shared_ptr<Mesh<Vertex3D>> ChunksRenderer::render(Chunk* chunk) {
        chunk->setModified(false);
        auto *mesh = m_renderer->renderVulkanMesh(chunk, m_level->chunksStorage);
        auto sptr = std::shared_ptr<Mesh<Vertex3D>>(mesh);
        m_meshes[glm::ivec2(chunk->x, chunk->z)] = sptr;
        return sptr;
    }

    void ChunksRenderer::unload(Chunk* chunk) {
        const auto found = m_meshes.find(glm::ivec2(chunk->x, chunk->z));
        if (found != m_meshes.end()) {
            m_meshes.erase(found);
        }
    }

    std::shared_ptr<Mesh<Vertex3D>> ChunksRenderer::getOrRender(Chunk* chunk) {
        const auto found = m_meshes.find(glm::ivec2(chunk->x, chunk->z));
        if (found != m_meshes.end() && !chunk->isModified()){
            return found->second;
        }
        return render(chunk);
    }

    std::shared_ptr<Mesh<Vertex3D>> ChunksRenderer::get(Chunk* chunk) {
        const auto found = m_meshes.find(glm::ivec2(chunk->x, chunk->z));
        if (found != m_meshes.end()) {
            return found->second;
        }
        return nullptr;
    }
} // vulkan