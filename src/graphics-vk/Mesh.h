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
#include "../typedefs.h"

namespace vulkan {

    struct vattr {
        ubyte size;
    };

    template<typename TVertex>
    class Mesh {
        static_assert(tools::is_same_of_types_v<TVertex, VertexMain, VertexLines, Vertex2D, VertexScreen>, "Only vertices");

        std::unique_ptr<Buffer> m_vertexBuffer = nullptr;
        std::unique_ptr<Buffer> m_indexBuffer = nullptr;
        size_t m_vertices;
        size_t m_indices;
    public:
        Mesh(const TVertex *vertexBuffer, size_t vertices, const int* indexBuffer, size_t indices, const vattr* attrs);
        Mesh(const TVertex *vertexBuffer, size_t vertices, const vattr* attrs) :
            Mesh(vertexBuffer, vertices, nullptr, 0, attrs) {}
        ~Mesh() = default;

        void reload(const TVertex *vertexBuffer, size_t vertices, const int *indexBuffer = nullptr, size_t indices = 0);
        void draw(unsigned int primitive);
        void draw();
    };

    template<typename TVertex>
    Mesh<TVertex>::Mesh(const TVertex *vertexBuffer, size_t vertices, const int *indexBuffer, size_t indices, const vattr* attrs) : m_vertices(vertices), m_indices(indices) {
        reload(vertexBuffer, vertices, indexBuffer, indices);
    }

    template<typename TVertex>
    void Mesh<TVertex>::reload(const TVertex *vertexBuffer, size_t vertices, const int *indexBuffer, size_t indices) {

        const VkDeviceSize vertexBufferSize = sizeof(TVertex) * vertices;
        const VkDeviceSize indexBufferSize = sizeof(int) * indices;

        if (m_vertexBuffer != nullptr && m_vertexBuffer->getSize() >= vertexBufferSize) {
            void *data = nullptr;
            m_vertexBuffer->mapMemory(&data);

            memcpy(data, vertexBuffer, vertexBufferSize);

            m_vertexBuffer->unmapMemory();
        }
        else {
            m_vertexBuffer.reset();

            if (vertexBuffer != nullptr && vertices > 0) {
                m_vertexBuffer = std::make_unique<Buffer>(vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

                void *data = nullptr;
                m_vertexBuffer->mapMemory(&data);

                memcpy(data, vertexBuffer, vertexBufferSize);

                m_vertexBuffer->unmapMemory();
            }
        }

        if (m_indexBuffer != nullptr && m_indexBuffer->getSize() >= indexBufferSize) {
            void *data = nullptr;
            m_indexBuffer->mapMemory(&data);

            memcpy(data, indexBuffer, indexBufferSize);

            m_indexBuffer->unmapMemory();
        }
        else {
            m_indexBuffer.reset();

            if (indexBuffer != nullptr && indices != 0) {
                m_indexBuffer = std::make_unique<Buffer>(indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

                void *data = nullptr;
                m_indexBuffer->mapMemory(&data);

                memcpy(data, indexBuffer, indexBufferSize);

                m_indexBuffer->unmapMemory();
            }
        }

        m_vertices = vertices;
        m_indices = indices;
    }

    template<typename TVertex>
    void Mesh<TVertex>::draw(unsigned int primitive) {
        if (m_vertexBuffer == nullptr) return;

        VkCommandBuffer commandBuffer = VulkanContext::get().getCurrentState().commandbuffer;
        const VkPrimitiveTopology primitiveTopology = tools::glTopologyToVulkan(primitive);

        vkCmdSetPrimitiveTopology(commandBuffer, primitiveTopology);

        constexpr VkDeviceSize offset = 0;
        if (m_indexBuffer != nullptr) {
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, *m_vertexBuffer, &offset);
            vkCmdBindIndexBuffer(commandBuffer, *m_indexBuffer, offset, VK_INDEX_TYPE_UINT32);
            vkCmdDrawIndexed(commandBuffer, m_indices, 1, 0, 0, 0);
        }
        else {
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, *m_vertexBuffer, &offset);
            vkCmdDraw(commandBuffer, m_vertices, 1, 0, 0);
        }
    }

    template<typename TVertex>
    void Mesh<TVertex>::draw() {
        draw(GL_TRIANGLES);
    }
}



#endif //VKMESH_H
