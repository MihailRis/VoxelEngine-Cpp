//
// Created by chelovek on 11/18/23.
//

#include "Allocator.h"

#include <array>

#include "Instance.h"
#include "Device.h"
#include "VulkanDefenitions.h"

namespace vulkan {
    Allocator::Allocator(const Instance &instance, const Device &device) : m_device(device) {
        VmaVulkanFunctions vulkanFunctions{};
        vulkanFunctions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
        vulkanFunctions.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;

        VmaAllocatorCreateInfo allocatorCreateInfo{};
        allocatorCreateInfo.vulkanApiVersion = instance.getApiVersion();
        allocatorCreateInfo.instance = instance;
        allocatorCreateInfo.physicalDevice = instance.getPhysicalDevice();
        allocatorCreateInfo.device = device;
        allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;

        vmaCreateAllocator(&allocatorCreateInfo, &m_allocator);
    }

    Allocator::operator VmaAllocator() const {
        return m_allocator;
    }

    void Allocator::createImage(VkExtent3D extent, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
        VkMemoryPropertyFlags properties, bool isCube, VkImage &image, VmaAllocation &allocation, uint32_t levelCount, uint32_t layerCount) const {
        VmaAllocationCreateInfo allocationCreateInfo{};
        allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
        allocationCreateInfo.flags = properties;

        VkImageCreateInfo imageCreateInfo{};
        imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
        imageCreateInfo.extent = extent;
        imageCreateInfo.mipLevels = levelCount;
        imageCreateInfo.arrayLayers = layerCount;
        imageCreateInfo.format = format;
        imageCreateInfo.tiling = tiling;
        imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageCreateInfo.usage = usage;
        imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageCreateInfo.flags = isCube ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0;

        CHECK_VK_FUNCTION(vmaCreateImage(m_allocator, &imageCreateInfo, &allocationCreateInfo, &image, &allocation, nullptr));
    }

    void Allocator::destroyImage(VkImage image, VmaAllocation allocation) const {
        vmaDestroyImage(m_allocator, image, allocation);
    }

    void Allocator::destroy() {
        vmaDestroyAllocator(m_allocator);
    }
} // vulkan