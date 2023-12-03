//
// Created by chelovek on 11/17/23.
//

#include "Framebuffers.h"

#include "Device.h"
#include "device/Swapchain.h"

Framebuffers::Framebuffers(Device& device, Swapchain& swapchain, VkRenderPass renderPass) : m_device(device) {
    const auto imageCount = swapchain.getImages().size();

    m_framebuffers.resize(imageCount);

    for (const auto &imageView : swapchain.getImageViews()) {
        auto framebuffer = device.createFramebuffer(renderPass, { imageView }, swapchain.getExtent());
        m_framebuffers.emplace_back(framebuffer);
    }
}

const std::vector<VkFramebuffer>& Framebuffers::getFramebuffers() const {
    return m_framebuffers;
}

void Framebuffers::destroy() {
    for (const auto &framebuffer : m_framebuffers) {
        vkDestroyFramebuffer(m_device, framebuffer, nullptr);
    }
}
