//
// Created by chelovek on 11/17/23.
//

#include "Swapchain.h"
#include <array>
#include <iostream>

#include "../Surface.h"
#include "../Device.h"
#include "../VulkanDefenitions.h"

Swapchain::Swapchain(Surface &surface, Device& device) : m_device(device) {
    const auto surfaceFormat = surface.getFormat();
    const auto surfaceCapabiliies = surface.getCapabilities();
    const auto presentMode = surface.getPresentMode();
    const auto extent = surfaceCapabiliies.currentExtent;

    u32 imageCount = surfaceCapabiliies.minImageCount + 1;
    if (surfaceCapabiliies.maxImageCount > 0 && imageCount > surfaceCapabiliies.maxImageCount)
        imageCount = surfaceCapabiliies.maxImageCount;

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    const std::array familyIndices = { device.getGraphis().getIndex(), device.getPresent().getIndex() };

    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;
    createInfo.pQueueFamilyIndices = nullptr;

    if (familyIndices[0] != familyIndices[1]) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = familyIndices.size();
        createInfo.pQueueFamilyIndices = familyIndices.data();
    }

    createInfo.preTransform = surfaceCapabiliies.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = true;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    CHECK_VK_FUNCTION(vkCreateSwapchainKHR(device, &createInfo, nullptr, &m_swapchain));

    imageCount = 0;
    vkGetSwapchainImagesKHR(device, m_swapchain, &imageCount, nullptr);
    m_images.resize(imageCount);
    vkGetSwapchainImagesKHR(device, m_swapchain, &imageCount, m_images.data());

    for (const auto &image : m_images) {
        auto imageView = device.createImageView(image, surfaceFormat.format, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT, {
            VK_COMPONENT_SWIZZLE_R,
            VK_COMPONENT_SWIZZLE_G,
            VK_COMPONENT_SWIZZLE_B,
            VK_COMPONENT_SWIZZLE_A
        }, 1, 1);

        m_imageViews.emplace_back(imageView);
    }

    m_extent2D = extent;
    m_format = surfaceFormat.format;
}

Swapchain::operator VkSwapchainKHR() const {
    return m_swapchain;
}

const std::vector<VkImage> &Swapchain::getImages() const {
    return m_images;
}

const std::vector<VkImageView> &Swapchain::getImageViews() const {
    return m_imageViews;
}

VkFormat Swapchain::getFormat() const {
    return m_format;
}


void Swapchain::destroy() {
    for (const auto &imageView : m_imageViews) {
        vkDestroyImageView(m_device, imageView, nullptr);
    }

    vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
}

VkExtent2D Swapchain::getExtent() const {
    return m_extent2D;
}

VkFormat Swapchain::getFormat() {
    return m_format;
}
