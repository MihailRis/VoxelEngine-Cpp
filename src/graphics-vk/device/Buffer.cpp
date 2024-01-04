//
// Created by chelovek on 11/19/23.
//

#include "Buffer.h"

#include <cstring>

#include "../VulkanContext.h"

namespace vulkan {
    Buffer::Buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VmaAllocationCreateFlags flags) : m_size(size) {
        const Allocator &allocator = VulkanContext::get().getAllocator();

        VkBufferCreateInfo bufferCreateInfo{};
        bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferCreateInfo.size = size;
        bufferCreateInfo.usage = usage;

        VmaAllocationCreateInfo allocationCreateInfo{};
        allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
        allocationCreateInfo.flags = flags;
        allocationCreateInfo.requiredFlags = properties;

        vmaCreateBuffer(allocator, &bufferCreateInfo, &allocationCreateInfo, &m_buffer, &m_allocation, &m_info);
    }

    Buffer::~Buffer() {
        destroy();
    }


    void Buffer::mapMemory(void **data) {
        const Allocator &allocator = VulkanContext::get().getAllocator();
        vmaMapMemory(allocator, m_allocation, data);
        m_mapped = true;
    }

    void Buffer::unmapMemory() {
        const Allocator &allocator = VulkanContext::get().getAllocator();
        vmaUnmapMemory(allocator, m_allocation);
        m_mapped = false;
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

    void Buffer::upluadDataToGpu(const void* data, size_t size) {
        const Device &device = VulkanContext::get().getDevice();

        VkCommandPoolCreateInfo commandPoolCreateInfo{};
        commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;

        VkCommandPool commandPool = VK_NULL_HANDLE;
        vkCreateCommandPool(device, &commandPoolCreateInfo, nullptr, &commandPool);

        VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
        commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferAllocateInfo.commandPool = commandPool;
        commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferAllocateInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
        vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &commandBuffer);

        Buffer stagingBuffer{size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT};
        void *stgData;

        stagingBuffer.mapMemory(&stgData);

        std::memcpy(stgData, data, size);

        stagingBuffer.unmapMemory();

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0;
        copyRegion.size = size;

        vkCmdCopyBuffer(commandBuffer, stagingBuffer, m_buffer, 1, &copyRegion);

        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(device.getGraphis(), 1, &submitInfo, nullptr);
        vkQueueWaitIdle(device.getGraphis());

        vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
        vkDestroyCommandPool(device, commandPool, nullptr);
    }

    void Buffer::destroy() {
        if (m_destroyed) return;
        if (m_mapped) unmapMemory();

        const Allocator &allocator = VulkanContext::get().getAllocator();
        vmaDestroyBuffer(allocator, m_buffer, m_allocation);
        m_destroyed = true;
    }

}
