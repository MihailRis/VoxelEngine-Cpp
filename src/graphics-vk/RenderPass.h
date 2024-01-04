//
// Created by chelovek on 11/19/23.
//

#ifndef RENDERPASS_H
#define RENDERPASS_H

#include <vulkan/vulkan.h>

class Device;

// This class maybe can used.
class RenderPass {
    VkDevice m_device;
    VkRenderPass m_renderPass = VK_NULL_HANDLE;
public:
    RenderPass(Device &device, VkFormat swapchainFormat, VkFormat depthFormat);

    operator VkRenderPass() const;

    void destroy();
};



#endif //RENDERPASS_H
