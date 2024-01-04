//
// Created by chelovek on 11/18/23.
//

#include "Image.h"

#include <iostream>

#include "../VulkanContext.h"
#include "../../constants.h"

VkFormat Image::selectSupportedFormat(const std::vector<VkFormat>& formats, VkImageTiling tiling,
    VkFormatFeatureFlags featureFlags) {
    return vulkan::VulkanContext::get().getDevice().selectSupportedFormat(formats, tiling, featureFlags);
}

Image::Image(VkExtent3D extent, VkFormat format, VkImageViewType viewType, VkImageAspectFlags aspectFlags, VkImageTiling tiling,
             VkImageUsageFlags usage, VkMemoryPropertyFlags properties, bool isCube, uint32_t levelCount, uint32_t layerCount)
    : m_format(format),
      m_extent3D(extent),
      m_layerCount(layerCount) {

    auto &device = vulkan::VulkanContext::get().getDevice();
    auto &allocator = vulkan::VulkanContext::get().getAllocator();

    allocator.createImage(extent, format, tiling, usage, properties, isCube, m_image, m_allocation, levelCount, layerCount);

    m_imageView = device.createImageView(m_image,
        format,
        viewType,
        aspectFlags,
        VkComponentMapping{
            VK_COMPONENT_SWIZZLE_R,
            VK_COMPONENT_SWIZZLE_G,
            VK_COMPONENT_SWIZZLE_B,
            VK_COMPONENT_SWIZZLE_A
        },
        levelCount,
        layerCount
    );
}

Image::~Image() {
    destroy();
}

Image::operator VkImage() const {
    return m_image;
}

VkImage Image::getImage() const {
    return m_image;
}

VkImageView Image::getView() const {
    return m_imageView;
}

VkSampler Image::getSampler() const {
    return m_sampler;
}

VkFormat Image::getFormat() const {
    return m_format;
}

uint32_t Image::getLayerCount() const {
    return m_layerCount;
}

void Image::destroy() {
    if (m_destroyed) return;

    auto &device = vulkan::VulkanContext::get().getDevice();
    auto &allocator = vulkan::VulkanContext::get().getAllocator();

    if (m_sampler != VK_NULL_HANDLE)
        vkDestroySampler(device, m_sampler, nullptr);

    vkDestroyImageView(device, m_imageView, nullptr);
    allocator.destroyImage(m_image, m_allocation);

    m_destroyed = true;
}
