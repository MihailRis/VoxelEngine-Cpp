//
// Created by chelovek on 11/19/23.
//

#ifndef BUFFER_H
#define BUFFER_H

#include <vulkan/vulkan.h>

#include "../Allocator.h"

namespace vulkan {
    class Buffer {
    protected:
        VkBuffer m_buffer = VK_NULL_HANDLE;
        VmaAllocation m_allocation = VK_NULL_HANDLE;
        uint64_t m_size;
        bool m_destroyed = false;
        bool m_mapped = false;
        VmaAllocationInfo m_info;
    public:
        Buffer(VkDeviceSize size, VkBufferUsageFlags usage,
            VkMemoryPropertyFlags properties,
            VmaAllocationCreateFlags flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT);
        ~Buffer();

        void mapMemory(void **data);
        void unmapMemory();

        uint64_t getSize() const;

        operator VkBuffer() const;
        operator const VkBuffer*() const;

        void upluadDataToGpu(const void *data, size_t size);

        void destroy();
    };
}




#endif //BUFFER_H
