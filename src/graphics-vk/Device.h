//
// Created by chelovek on 11/16/23.
//

#ifndef DEVICE_H
#define DEVICE_H

#include <vector>
#include <vulkan/vulkan_core.h>

#include "../util/Noncopybale.h"
#include "device/Queue.h"

class Instance;

class Device : Noncopybale {
    VkDevice m_device = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;

    Queue m_graphics;
    Queue m_present;

public:
    Device() = default;
    Device(Instance &instance, VkSurfaceKHR surface);

    operator VkDevice() const;

    const Queue &getGraphis() const;
    const Queue &getPresent() const;

    VkImageView createImageView(VkImage image, VkFormat format, VkImageViewType viewType, VkImageAspectFlags aspectFlags, VkComponentMapping components) const;
    VkSampler createSampler(VkFilter filter, VkSamplerAddressMode addressMode, bool anisotropic) const;
    VkRenderPass createRenderPass(VkFormat swapchainFormat, VkFormat depthFormat) const;
    VkFramebuffer createFramebuffer(VkRenderPass renderPass, const std::vector<VkImageView> &imageViews, VkExtent2D extent) const;
    VkFormat selectSupportedFormat(const std::vector<VkFormat> &formats, VkImageTiling tiling, VkFormatFeatureFlags features) const;
    VkCommandPool createCommadPool() const;
    VkCommandBuffer createCommandBuffer(VkCommandPool commandPool) const;

    void waitIdle() const;
    void destroy(VkCommandPool commandPool);
    void destroy();
};



#endif //DEVICE_H
