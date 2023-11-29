//
// Created by chelovek on 11/29/23.
//

#ifndef BLOCKRENDERER_H
#define BLOCKRENDERER_H

#include "Mesh.h"
#include "Vertices.h"
#include "../content/Content.h"

class ChunksStorage;
struct voxel;
class UVRegion;
class Chunk;
class VoxelsVolume;
class ContentGfxCache;
struct EngineSettings;
class Content;

namespace vulkan {

    class BlocksRenderer {
        const Content *m_content;
        VertexMain *m_vertexBuffer;
        int *m_indexBuffer;
        size_t m_vertexOffset = 0;
        size_t m_indexOffset = 0, m_indexSize = 0;
        size_t m_capacity = 0;

        bool overflow = false;

        const Chunk *m_chunk = nullptr;
        VoxelsVolume *m_voxelsBuffer;

        const Block *const *m_blockDefsCache;
        const ContentGfxCache *m_cache;
        const EngineSettings& m_settings;

        void vertex(const glm::vec3& coord, float u, float v, const glm::vec4& light);
        void index(int a, int b, int c, int d, int e, int f);

        void face(const glm::vec3& coord, float w, float h,
            const glm::vec3& axisX,
            const glm::vec3& axisY,
            const UVRegion& region,
            const glm::vec4(&lights)[4],
            const glm::vec4& tint);

        void face(const glm::vec3& coord, float w, float h,
            const glm::vec3& axisX,
            const glm::vec3& axisY,
            const UVRegion& region,
            const glm::vec4(&lights)[4],
            const glm::vec4& tint,
            bool rotated);

        void face(const glm::vec3& coord, float w, float h,
            const glm::vec3& axisX,
            const glm::vec3& axisY,
            const UVRegion& region,
            const glm::vec4(&lights)[4]) {
            face(coord, w, h, axisX, axisY, region, lights, glm::vec4(1.0f));
        }

        void cube(const glm::vec3& coord, const glm::vec3& size, const UVRegion(&faces)[6]);
        void blockCube(int x, int y, int z, const glm::vec3& size, const UVRegion(&faces)[6], ubyte group);
        void blockCubeShaded(int x, int y, int z, const glm::vec3& size, const UVRegion(&faces)[6], const Block* block, ubyte states);
        void blockXSprite(int x, int y, int z, const glm::vec3& size, const UVRegion& face1, const UVRegion& face2, float spread);

        bool isOpenForLight(int x, int y, int z) const;
        bool isOpen(int x, int y, int z, ubyte group) const;

        glm::vec4 pickLight(int x, int y, int z) const;
        glm::vec4 pickSoftLight(int x, int y, int z, const glm::ivec3& right, const glm::ivec3& up) const;
        void render(const voxel* voxels, int atlas_size);
    public:
        BlocksRenderer(size_t capacity, const Content* content, const ContentGfxCache* cache, const EngineSettings& settings);
        virtual ~BlocksRenderer();

        Mesh<VertexMain>* render(const Chunk* chunk, int atlas_size, const ChunksStorage* chunks);
        VoxelsVolume* getVoxelsBuffer() const;
    };

} // vulkan

#endif //BLOCKRENDERER_H
