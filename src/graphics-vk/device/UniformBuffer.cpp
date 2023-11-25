//
// Created by chelovek on 11/23/23.
//

#include "UniformBuffer.h"

namespace vulkan {
    UniformBuffer::UniformBuffer(size_t size) : Buffer(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
    }

    VkDescriptorBufferInfo UniformBuffer::getBufferInfo() const {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = m_buffer;
        bufferInfo.offset = 0;
        bufferInfo.range = m_size;

        return bufferInfo;
    }
} // vulkan