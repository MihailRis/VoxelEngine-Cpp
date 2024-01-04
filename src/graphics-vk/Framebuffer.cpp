//
// Created by chelovek on 11/18/23.
//

#include "Framebuffer.h"

#include "Device.h"

Framebuffer::Framebuffer(Device &device,
    VkRenderPass renderPass,
    const std::vector<VkImageView> &imageViews,
    VkExtent2D extent)
    : m_device(device) {
    m_framebuffer = device.createFramebuffer(renderPass, imageViews, extent);
}

Framebuffer::operator VkFramebuffer() const {
    return m_framebuffer;
}

void Framebuffer::destroy() {
    vkDestroyFramebuffer(m_device, m_framebuffer, nullptr);
}
