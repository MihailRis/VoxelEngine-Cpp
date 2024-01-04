//
// Created by chelovek on 11/23/23.
//

#include "UniformBuffer.h"

namespace vulkan {
    UniformBuffer::UniformBuffer(size_t size)
        : Buffer(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT),
          m_baseSize(size),
          m_isDynamic(false) {
    }

    UniformBuffer::UniformBuffer(size_t size, size_t count)
        : Buffer(size * count, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT),
          m_baseSize(size),
          m_isDynamic(true) {
    }

    initializers::UniformBufferInfo UniformBuffer::getBufferInfo() const {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = m_buffer;
        bufferInfo.offset = 0;
        bufferInfo.range = m_baseSize;

        const initializers::UniformBufferInfo uniformBufferInfo = { m_isDynamic, bufferInfo };

        return uniformBufferInfo;
    }
} // vulkan