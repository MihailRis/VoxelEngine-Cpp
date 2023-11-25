//
// Created by chelovek on 11/18/23.
//

#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <vk_mem_alloc.h>

#include "../util/Noncopybale.h"

class Instance;
class Device;

namespace vulkan {

    class Allocator : Noncopybale {
        VmaAllocator m_allocator = VK_NULL_HANDLE;
    public:
        Allocator(const Instance &instance, const Device &device);

        operator VmaAllocator() const;

        void createImage(VkExtent3D extent, VkFormat format, VkImageTiling tiling,
             VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage &image, VmaAllocation &allocation) const;

        void destroyImage(VkImage image, VmaAllocation allocation) const;

        void destroy();
    };

} // vulkan

#endif //ALLOCATOR_H
