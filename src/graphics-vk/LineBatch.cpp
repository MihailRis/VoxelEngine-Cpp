//
// Created by chelovek on 11/29/23.
//

#include "LineBatch.h"

#include <numeric>

namespace vulkan {
    LineBatch::LineBatch(size_t capacity) : m_capacity(capacity) {
        m_buffer = new VertexLines[capacity];
        std::memset(m_buffer, 0, capacity * sizeof(VertexLines));
        m_mesh = new Mesh(m_buffer, capacity);
    }

    LineBatch::~LineBatch() {
        delete[] m_buffer;
        delete m_mesh;
    }

    void LineBatch::line(float x1, float y1, float z1, float x2, float y2, float z2, float r, float g, float b,
        float a) {
        if (m_index >= m_capacity)
            return;

        m_buffer[m_index++] = VertexLines{{x1, y1, z1}, {r, g, b, a}};
        m_buffer[m_index++] = VertexLines{{x2, y2, z2}, {r, g, b, a}};
    }

    void LineBatch::box(float x, float y, float z, float w, float h, float d, float r, float g, float b, float a) {
        w *= 0.5f;
        h *= 0.5f;
        d *= 0.5f;

        line(x-w, y-h, z-d, x+w, y-h, z-d, r,g,b,a);
        line(x-w, y+h, z-d, x+w, y+h, z-d, r,g,b,a);
        line(x-w, y-h, z+d, x+w, y-h, z+d, r,g,b,a);
        line(x-w, y+h, z+d, x+w, y+h, z+d, r,g,b,a);

        line(x-w, y-h, z-d, x-w, y+h, z-d, r,g,b,a);
        line(x+w, y-h, z-d, x+w, y+h, z-d, r,g,b,a);
        line(x-w, y-h, z+d, x-w, y+h, z+d, r,g,b,a);
        line(x+w, y-h, z+d, x+w, y+h, z+d, r,g,b,a);

        line(x-w, y-h, z-d, x-w, y-h, z+d, r,g,b,a);
        line(x+w, y-h, z-d, x+w, y-h, z+d, r,g,b,a);
        line(x-w, y+h, z-d, x-w, y+h, z+d, r,g,b,a);
        line(x+w, y+h, z-d, x+w, y+h, z+d, r,g,b,a);
    }

    void LineBatch::render() {
        if (m_index == 0) return;
        m_mesh->reload(m_buffer, m_index);
        m_mesh->draw({0, m_index}, 0x0001);
        m_index = 0;
    }

    void LineBatch::lineWidth(float width) {
        VkCommandBuffer commandBuffer = VulkanContext::get().getCurrentState().commandbuffer;
        if (commandBuffer == VK_NULL_HANDLE) return;
        vkCmdSetLineWidth(commandBuffer, width);
    }
} // vulkan