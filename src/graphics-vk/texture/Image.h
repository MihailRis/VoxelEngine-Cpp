//
// Created by chelovek on 11/18/23.
//

#ifndef IMAGE_H
#define IMAGE_H

#include <vector>
#include <vulkan/vulkan.h>

#include "../Allocator.h"

class Image {
    VmaAllocation m_allocation = VK_NULL_HANDLE;
    VkFormat m_format;
    bool m_destroyed = false;
protected:
    VkExtent3D m_extent3D;
    VkImage m_image = VK_NULL_HANDLE;
    VkImageView m_imageView = VK_NULL_HANDLE;
    VkSampler m_sampler = VK_NULL_HANDLE;
    uint32_t m_layerCount = 0;

    static VkFormat selectSupportedFormat(const std::vector<VkFormat> &formats, VkImageTiling tiling, VkFormatFeatureFlags featureFlags);
public:
    Image() = default;
    Image(VkExtent3D extent, VkFormat format, VkImageViewType viewType, VkImageAspectFlags aspectFlags, VkImageTiling tiling,
          VkImageUsageFlags usage, VkMemoryPropertyFlags properties, bool isCube = false, uint32_t levelCount = 1, uint32_t layerCount = 1);

    virtual ~Image();

    operator VkImage() const;

    VkImage getImage() const;

    VkImageView getView() const;

    VkSampler getSampler() const;

    VkFormat getFormat() const;

    uint32_t getLayerCount() const;

    void destroy();

    static uint32_t getWidth(const Image &image) { return image.m_extent3D.width; }
    static uint32_t getHeight(const Image &image) { return image.m_extent3D.height; }
};



#endif //IMAGE_H
