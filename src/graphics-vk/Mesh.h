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
        static_assert(tools::is_same_of_types_v<TVertex, Vertex3D, VertexLine, Vertex2D, VertexBackSkyGen>, "Only vertices");

        std::unique_ptr<Buffer> m_stagingVertexBuffer = nullptr;
        std::unique_ptr<Buffer> m_stagingIndexBuffer = nullptr;

        std::unique_ptr<Buffer> m_vertexBuffer = nullptr;
        std::unique_ptr<Buffer> m_indexBuffer = nullptr;

        size_t m_vertices = 0;
        size_t m_indices = 0;
    public:
        Mesh() = default;
        Mesh(const TVertex *vertexBuffer, size_t vertices, const int* indexBuffer, size_t indices);
        Mesh(const TVertex *vertexBuffer, size_t vertices) :
            Mesh(vertexBuffer, vertices, nullptr, 0) {}
        ~Mesh() = default;

        void bind();
        void reload(const TVertex *vertexBuffer, size_t vertices, const int *indexBuffer = nullptr, size_t indices = 0);
        void draw(const VertexOffset &offset, unsigned int primitive);
        void draw(const VertexOffset &offset);

        void mapVertex(TVertex **data);
        void mapIndex(int **data);

        void unmapVertex();
        void unmapIndex();
    };

    template<typename TVertex>
    Mesh<TVertex>::Mesh(const TVertex *vertexBuffer, size_t vertices, const int *indexBuffer, size_t indices) : m_vertices(vertices), m_indices(indices) {
        m_stagingVertexBuffer = std::make_unique<Buffer>(vertices * sizeof(TVertex), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        m_vertexBuffer = std::make_unique<Buffer>(vertices * sizeof(TVertex), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0);

        if (indices > 0) {
            m_stagingIndexBuffer = std::make_unique<Buffer>(indices * sizeof(int), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            m_indexBuffer = std::make_unique<Buffer>(indices * sizeof(int), VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0);
        }

        reload(vertexBuffer, vertices, indexBuffer, indices);
    }

    template<typename TVertex>
    void Mesh<TVertex>::bind() {
        constexpr VkDeviceSize offsetSize = 0;
        const auto commandBuffer = VulkanContext::get().getCurrentState().commandbuffer;
        if (m_vertexBuffer != nullptr) {
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, *m_vertexBuffer, &offsetSize);
        }
        if (m_indexBuffer != nullptr) {
            vkCmdBindIndexBuffer(commandBuffer, *m_indexBuffer, offsetSize, VK_INDEX_TYPE_UINT32);
        }
    }

    template<typename TVertex>
    void Mesh<TVertex>::reload(const TVertex *vertexBuffer, size_t vertices, const int *indexBuffer, size_t indices) {

        const VkDeviceSize vertexBufferSize = sizeof(TVertex) * vertices;
        const VkDeviceSize indexBufferSize = sizeof(int) * indices;

        if (m_stagingVertexBuffer != nullptr && vertexBuffer != nullptr && m_stagingVertexBuffer->getSize() >= vertexBufferSize && vertices > 0) {
            void *data = nullptr;
            m_stagingVertexBuffer->mapMemory(&data);

            memcpy(data, vertexBuffer, vertexBufferSize);

            m_stagingVertexBuffer->unmapMemory();
        }

        if (m_stagingIndexBuffer != nullptr && indexBuffer != nullptr && m_stagingIndexBuffer->getSize() >= indexBufferSize && indices > 0) {
            void *data = nullptr;
            m_stagingIndexBuffer->mapMemory(&data);

            memcpy(data, indexBuffer, indexBufferSize);

            m_stagingIndexBuffer->unmapMemory();
        }

        VulkanContext::get().immediateSubmit([=](VkCommandBuffer commandBuffer) {

            if (m_stagingVertexBuffer == nullptr) return;

            VkBufferCopy vertexBufferCopy{};
            vertexBufferCopy.dstOffset = 0;
            vertexBufferCopy.srcOffset = 0;
            vertexBufferCopy.size = m_stagingVertexBuffer->getSize();

            vkCmdCopyBuffer(commandBuffer, *m_stagingVertexBuffer, *m_vertexBuffer, 1, &vertexBufferCopy);

            if (m_stagingIndexBuffer == nullptr) return;

            VkBufferCopy indexBufferCopy{};
            indexBufferCopy.dstOffset = 0;
            indexBufferCopy.srcOffset = 0;
            indexBufferCopy.size = m_stagingIndexBuffer->getSize();

            vkCmdCopyBuffer(commandBuffer, *m_stagingIndexBuffer, *m_indexBuffer, 1, &indexBufferCopy);
        });

        m_vertices = vertices;
        m_indices = indices;
    }

    template<typename TVertex>
    void Mesh<TVertex>::draw(const VertexOffset &offset, unsigned int primitive) {
        if (m_stagingVertexBuffer == nullptr) return;

        const auto commandBuffer = VulkanContext::get().getCurrentState().commandbuffer;

        if (m_stagingIndexBuffer != nullptr || m_indices > 0) {
            vkCmdDrawIndexed(commandBuffer, m_indices, 1, 0, 0, 0);
        }
        else if (m_vertices > 0) {
            vkCmdDraw(commandBuffer, offset.count, 1, offset.offset, 0);
        }
    }

    template<typename TVertex>
    void Mesh<TVertex>::draw(const VertexOffset &offset) {
        draw(offset, GL_TRIANGLES);
    }

    template<typename TVertex>
    void Mesh<TVertex>::mapVertex(TVertex **data) {
        m_stagingVertexBuffer->mapMemory(reinterpret_cast<void**>(data));
    }

    template<typename TVertex>
    void Mesh<TVertex>::mapIndex(int **data) {
        m_stagingIndexBuffer->mapMemory(reinterpret_cast<void**>(data));
    }

    template<typename TVertex>
    void Mesh<TVertex>::unmapVertex() {
        m_stagingVertexBuffer->unmapMemory();
    }

    template<typename TVertex>
    void Mesh<TVertex>::unmapIndex() {
        m_stagingIndexBuffer->unmapMemory();
    }
}



#endif //VKMESH_H
