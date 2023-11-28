//
// Created by chelovek on 11/19/23.
//

#ifndef VKMESH_H
#define VKMESH_H

#include <memory>

#include "Tools.h"
#include "Vertices.h"
#include "VulkanContext.h"
#include "device/Buffer.h"

namespace vulkan {

    struct VertexOffset {
        size_t offset;
        size_t count;
    };

    template<typename TVertex>
    class Mesh {
        static_assert(tools::is_same_of_types_v<TVertex, VertexMain, VertexLines, Vertex2D, VertexBackSkyGen>, "Only vertices");

        std::unique_ptr<Buffer> m_vertexBuffer = nullptr;
        std::unique_ptr<Buffer> m_indexBuffer = nullptr;
        size_t m_vertices;
        size_t m_indices;
    public:
        Mesh(const TVertex *vertexBuffer, size_t vertices, const int* indexBuffer, size_t indices);
        Mesh(const TVertex *vertexBuffer, size_t vertices) :
            Mesh(vertexBuffer, vertices, nullptr, 0) {}
        ~Mesh() = default;

        void reload(const TVertex *vertexBuffer, size_t vertices, const int *indexBuffer = nullptr, size_t indices = 0);
        void draw(const VertexOffset &offset, unsigned int primitive);
        void draw(const VertexOffset &offset);
    };

    template<typename TVertex>
    Mesh<TVertex>::Mesh(const TVertex *vertexBuffer, size_t vertices, const int *indexBuffer, size_t indices) : m_vertices(vertices), m_indices(indices) {
        m_vertexBuffer = std::make_unique<Buffer>(vertices * sizeof(TVertex), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        reload(vertexBuffer, vertices, indexBuffer, indices);
    }

    template<typename TVertex>
    void Mesh<TVertex>::reload(const TVertex *vertexBuffer, size_t vertices, const int *indexBuffer, size_t indices) {

        const VkDeviceSize vertexBufferSize = sizeof(TVertex) * vertices;
        const VkDeviceSize indexBufferSize = sizeof(int) * indices;

        if (m_vertexBuffer != nullptr && m_vertexBuffer->getSize() > vertexBufferSize && vertices > 0) {
            void *data = nullptr;
            m_vertexBuffer->mapMemory(&data);

            memcpy(data, vertexBuffer, vertexBufferSize);

            m_vertexBuffer->unmapMemory();
        }

        if (m_indexBuffer != nullptr && m_indexBuffer->getSize() > indexBufferSize && indices > 0) {
            void *data = nullptr;
            m_indexBuffer->mapMemory(&data);

            memcpy(data, indexBuffer, indexBufferSize);

            m_indexBuffer->unmapMemory();
        }

        m_vertices = vertices;
        m_indices = indices;
    }

    template<typename TVertex>
    void Mesh<TVertex>::draw(const VertexOffset &offset, unsigned int primitive) {
        if (m_vertexBuffer == nullptr) return;

        VkCommandBuffer commandBuffer = VulkanContext::get().getCurrentState().commandbuffer;
        const VkPrimitiveTopology primitiveTopology = tools::glTopologyToVulkan(primitive);

        vkCmdSetPrimitiveTopology(commandBuffer, primitiveTopology);

        constexpr VkDeviceSize offsetSize = 0;
        if (m_indexBuffer != nullptr || m_indices > 0) {
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, *m_vertexBuffer, &offsetSize);
            vkCmdBindIndexBuffer(commandBuffer, *m_indexBuffer, offsetSize, VK_INDEX_TYPE_UINT32);
            vkCmdDrawIndexed(commandBuffer, m_indices, 1, 0, 0, 0);
        }
        else if (m_vertices > 0) {
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, *m_vertexBuffer, &offsetSize);
            vkCmdDraw(commandBuffer, offset.count, 1, offset.offset, 0);
        }
    }

    template<typename TVertex>
    void Mesh<TVertex>::draw(const VertexOffset &offset) {
        draw(offset, GL_TRIANGLES);
    }
}



#endif //VKMESH_H
