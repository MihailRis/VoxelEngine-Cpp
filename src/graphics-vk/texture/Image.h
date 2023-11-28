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

    static VkFormat selectSupportedFormat(const std::vector<VkFormat> &formats, VkImageTiling tiling, VkFormatFeatureFlags featureFlags);
public:
    Image() = default;
    Image(VkExtent3D extent, VkFormat format, VkImageViewType viewType, VkImageAspectFlags aspectFlags, VkImageTiling tiling,
          VkImageUsageFlags usage, VkMemoryPropertyFlags properties);

    virtual ~Image();

    operator VkImage() const;

    VkImage getImage() const;

    VkImageView getView() const;

    VkSampler getSampler() const;

    VkFormat getFormat() const;

    void destroy();
};



#endif //IMAGE_H
