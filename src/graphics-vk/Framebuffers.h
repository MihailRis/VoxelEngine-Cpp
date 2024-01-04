//
// Created by chelovek on 11/17/23.
//

#ifndef FRAMEBUFFERS_H
#define FRAMEBUFFERS_H

#include <vector>
#include <vulkan/vulkan.h>

class Device;
class Swapchain;

class Framebuffers {
    VkDevice m_device;
    std::vector<VkFramebuffer> m_framebuffers;
public:
    Framebuffers(Device &device, Swapchain &swapchain, VkRenderPass renderPass);

    const std::vector<VkFramebuffer> &getFramebuffers() const;

    void destroy();
};



#endif //FRAMEBUFFERS_H
