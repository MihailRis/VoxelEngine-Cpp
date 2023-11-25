//
// Created by chelovek on 11/19/23.
//

#include "Buffer.h"

#include "../VulkanContext.h"

namespace vulkan {
    Buffer::Buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) : m_size(size) {
        auto &allocator = VulkanContext::get().getAllocator();

        VkBufferCreateInfo bufferCreateInfo{};
        bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferCreateInfo.size = size;
        bufferCreateInfo.usage = usage;

        VmaAllocationCreateInfo allocationCreateInfo{};
        allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
        allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
        allocationCreateInfo.requiredFlags = properties;

        vmaCreateBuffer(allocator, &bufferCreateInfo, &allocationCreateInfo, &m_buffer, &m_allocation, nullptr);
    }

    Buffer::~Buffer() {
        destroy();
    }


    void Buffer::mapMemory(void **data) const {
        auto &allocator = VulkanContext::get().getAllocator();
        vmaMapMemory(allocator, m_allocation, data);
    }

    void Buffer::unmapMemory() const {
        auto &allocator = VulkanContext::get().getAllocator();
        vmaUnmapMemory(allocator, m_allocation);
    }

    uint64_t Buffer::getSize() const {
        return m_size;
    }

    Buffer::operator VkBuffer() const {
        return m_buffer;
    }

    Buffer::operator const VkBuffer*() const {
        return &m_buffer;
    }

    void Buffer::destroy() {
        if (m_destroyed) return;
        auto &allocator = VulkanContext::get().getAllocator();
        vmaDestroyBuffer(allocator, m_buffer, m_allocation);
        m_destroyed = true;
    }

}
