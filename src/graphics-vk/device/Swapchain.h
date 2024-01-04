//
// Created by chelovek on 11/17/23.
//

#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#include <vector>
#include <vulkan/vulkan.h>

class Instance;
class Device;
class Surface;

class Swapchain {
    VkDevice m_device;
    VkSwapchainKHR m_swapchain;
    std::vector<VkImage> m_images;
    std::vector<VkImageView> m_imageViews;

    VkFormat m_format{};
    VkExtent2D m_extent2D{};
public:
    Swapchain() = default;
    Swapchain(Surface &surface, Device &device);

    operator VkSwapchainKHR() const;

    const std::vector<VkImage> &getImages() const;
    const std::vector<VkImageView> &getImageViews() const;
    VkFormat getFormat() const;

    void destroy();

    VkExtent2D getExtent() const;

    VkFormat getFormat();
};



#endif //SWAPCHAIN_H
