//
// Created by chelovek on 11/18/23.
//

#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <vulkan/vulkan.hpp>

class Device;

class Framebuffer {
    VkDevice m_device;
    VkFramebuffer m_framebuffer;
public:
    Framebuffer(Device &device, VkRenderPass renderPass, const std::vector<VkImageView> &imageViews, VkExtent2D extent);

    operator VkFramebuffer() const;

    void destroy();
};



#endif //FRAMEBUFFER_H
