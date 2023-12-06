#include "VulkanContext.h"

#include <iostream>
#include <array>

#include "Tools.h"
#include "VulkanDefenitions.h"
#include "../window/Window.h"
#include "device/GraphicsPipeline.h"
#include "uniforms/ApplyUniform.h"
#include "uniforms/BackgroundUniform.h"
#include "uniforms/FogUniform.h"
#include "uniforms/ProjectionViewUniform.h"
#include "uniforms/SkyboxUniform.h"
#include "uniforms/StateUniform.h"

constexpr uint32_t DESCRIPTOR_SET_COUNT = 1000;

namespace vulkan {

    PFN_vkCmdPushDescriptorSetKHR vkCmdPushDescriptorSetKhr = nullptr;

    bool VulkanContext::vulkanEnabled = false;

    void UniformBuffersHolder::initBuffers() {
        m_buffers.emplace_back(std::make_unique<UniformBuffer>(sizeof(StateUniform)));
        m_buffers.emplace_back(std::make_unique<UniformBuffer>(sizeof(FogUniform)));
        m_buffers.emplace_back(std::make_unique<UniformBuffer>(sizeof(ProjectionViewUniform)));
        m_buffers.emplace_back(std::make_unique<UniformBuffer>(sizeof(BackgroundUniform)));
        m_buffers.emplace_back(std::make_unique<UniformBuffer>(sizeof(SkyboxUniform)));
        m_buffers.emplace_back(std::make_unique<UniformBuffer>(sizeof(ApplyUniform)));
    }

    UniformBuffer* UniformBuffersHolder::operator[](Type index) const {
        return m_buffers.at(index).get();
    }

    void UniformBuffersHolder::destroy() {
        for (auto &uniformBuffer : m_buffers) {
            uniformBuffer.reset();
        }
    }

    VulkanContext::VulkanContext()
        : m_instance(Instance::create()),
          m_surface(m_instance.createSurface()),
          m_device(m_instance, m_surface),
          m_swapchain(std::make_unique<Swapchain>(m_surface, m_device)),
          m_allocator(m_instance, m_device) {
        vkCmdPushDescriptorSetKhr = reinterpret_cast<PFN_vkCmdPushDescriptorSetKHR>(vkGetDeviceProcAddr(m_device, "vkCmdPushDescriptorSetKHR"));
    }

    void VulkanContext::initDescriptorPool() {
        const std::vector<VkDescriptorPoolSize> descriptorPoolSizes = {
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, DESCRIPTOR_SET_COUNT },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, DESCRIPTOR_SET_COUNT }
        };

        VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{};
        descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptorPoolCreateInfo.maxSets = DESCRIPTOR_SET_COUNT;
        descriptorPoolCreateInfo.poolSizeCount = descriptorPoolSizes.size();
        descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizes.data();
        descriptorPoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;

        CHECK_VK(vkCreateDescriptorPool(m_device, &descriptorPoolCreateInfo, nullptr, &m_descriptorPool));
    }

    void VulkanContext::initDepth() {
        const auto swapChainExtent = m_swapchain->getExtent();
        m_imageDepth = std::make_unique<ImageDepth>(VkExtent3D{swapChainExtent.width, swapChainExtent.height, 1});
    }

    void VulkanContext::initUploadContext() {
        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        // fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        VkCommandPoolCreateInfo commandPoolCreateInfo{};
        commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        commandPoolCreateInfo.queueFamilyIndex = m_device.getGraphis().getIndex();

        VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
        commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferAllocateInfo.commandBufferCount = 1;

        CHECK_VK(vkCreateCommandPool(m_device, &commandPoolCreateInfo, nullptr, &m_uploadContext.commandPool));

        commandBufferAllocateInfo.commandPool = m_uploadContext.commandPool;

        CHECK_VK(vkAllocateCommandBuffers(m_device, &commandBufferAllocateInfo, &m_uploadContext.commandBuffer));

        CHECK_VK(vkCreateFence(m_device, &fenceInfo, nullptr, &m_uploadContext.uploadFence));
    }

    void VulkanContext::initFrameDatas() {
        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        // fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        VkCommandPoolCreateInfo commandPoolCreateInfo{};
        commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        commandPoolCreateInfo.queueFamilyIndex = m_device.getGraphis().getIndex();

        VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
        commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferAllocateInfo.commandBufferCount = 1;

        for (auto &m_frameData : m_frameDatas) {
            CHECK_VK(vkCreateCommandPool(m_device, &commandPoolCreateInfo, nullptr, &m_frameData.commandPool));

            commandBufferAllocateInfo.commandPool = m_frameData.commandPool;

            CHECK_VK(vkAllocateCommandBuffers(m_device, &commandBufferAllocateInfo, &m_frameData.screenCommandBuffer));
            CHECK_VK(vkAllocateCommandBuffers(m_device, &commandBufferAllocateInfo, &m_frameData.guiCommandBuffer));

            CHECK_VK(vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_frameData.presentSemaphore));
            CHECK_VK(vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_frameData.renderSemaphore));
            CHECK_VK(vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_frameData.uiRenderSemaphore));
            CHECK_VK(vkCreateFence(m_device, &fenceInfo, nullptr, &m_frameData.renderFence));
        }
    }

    void VulkanContext::initUniformBuffers() {
        m_uniformBuffersHolder.initBuffers();
    }

    void VulkanContext::destroy() {
        vkDeviceWaitIdle(m_device);

        for (auto &frameData : m_frameDatas) {
            vkDestroyCommandPool(m_device, frameData.commandPool, nullptr);

            vkDestroyFence(m_device, frameData.renderFence, nullptr);
            vkDestroySemaphore(m_device, frameData.presentSemaphore, nullptr);
            vkDestroySemaphore(m_device, frameData.renderSemaphore, nullptr);
            vkDestroySemaphore(m_device, frameData.uiRenderSemaphore, nullptr);
        }

        m_uniformBuffersHolder.destroy();
        vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);
        m_imageDepth->destroy();
        m_allocator.destroy();
        m_swapchain->destroy();
        m_device.destroy();
        m_surface.destroy();
        m_instance.destroy();
    }

    const Device& VulkanContext::getDevice() const {
        return m_device;
    }

    const Allocator &VulkanContext::getAllocator() const {
        return m_allocator;
    }

    const State &VulkanContext::getCurrentState() const {
        return m_state;
    }

    const Swapchain& VulkanContext::getSwapchain() const {
        return *m_swapchain;
    }

    const ImageDepth& VulkanContext::getDepth() const {
        return *m_imageDepth;
    }

    VkDescriptorPool VulkanContext::getDescriptorPool() const {
        return m_descriptorPool;
    }

    void VulkanContext::immediateSubmit(std::function<void(VkCommandBuffer)>&& function) const {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        CHECK_VK(vkBeginCommandBuffer(m_uploadContext.commandBuffer, &beginInfo));

        function(m_uploadContext.commandBuffer);

        CHECK_VK(vkEndCommandBuffer(m_uploadContext.commandBuffer));

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_uploadContext.commandBuffer;

        CHECK_VK(vkQueueSubmit(m_device.getGraphis(), 1, &submitInfo, m_uploadContext.uploadFence));

        vkWaitForFences(m_device, 1, &m_uploadContext.uploadFence, VK_TRUE, UINT64_MAX);
        vkResetFences(m_device, 1, &m_uploadContext.uploadFence);

        vkResetCommandPool(m_device, m_uploadContext.commandPool, 0);
    }

    UniformBuffer* VulkanContext::getUniformBuffer(UniformBuffersHolder::Type type) {
        return m_uniformBuffersHolder[type];
    }

    void VulkanContext::recreateSwapChain() {
        m_swapchain->destroy();
        m_swapchain.reset();
        m_swapchain = std::make_unique<Swapchain>(m_surface, m_device);
    }

    void VulkanContext::updateState(GraphicsPipeline* pipeline) {
        m_state.pipeline = pipeline;
    }

    void VulkanContext::updateStateCommandBuffer(VkCommandBuffer commandBuffer) {
        m_state.commandbuffer = commandBuffer;
    }

    // void VulkanContext::beginDrawToImage(const Image& image, float r, float g, float b, VkAttachmentLoadOp loadOp) {
    //     CHECK_VK(vkWaitForFences(m_device, 1, &m_renderFence, VK_TRUE, UINT64_MAX));
    //     CHECK_VK(vkResetFences(m_device, 1, &m_renderFence));
    //
    //     CHECK_VK(vkAcquireNextImageKHR(m_device, *m_swapchain, UINT64_MAX, m_presentSemaphore, VK_NULL_HANDLE, &m_currentImage));
    //
    //     CHECK_VK(vkResetCommandBuffer(m_frameDatas[m_currentFrame].screenCommandBuffer, 0));
    //
    //     VkCommandBufferBeginInfo commandBufferBeginInfo{};
    //     commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    //     commandBufferBeginInfo.flags = 0;
    //     commandBufferBeginInfo.pInheritanceInfo = nullptr;
    //
    //     CHECK_VK(vkBeginCommandBuffer(m_frameDatas[m_currentFrame].screenCommandBuffer, &commandBufferBeginInfo));
    //
    //     tools::insertImageMemoryBarrier(m_frameDatas[m_currentFrame].screenCommandBuffer,
    //         image.getImage(),
    //         VK_ACCESS_MEMORY_READ_BIT,
    //         VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
    //         VK_IMAGE_LAYOUT_UNDEFINED,
    //         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    //         VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
    //         VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    //         VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });
    //
    //     tools::insertImageMemoryBarrier(m_frameDatas[m_currentFrame].screenCommandBuffer,
    //         m_imageDepth->getImage(),
    //         0,
    //         VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
    //         VK_IMAGE_LAYOUT_UNDEFINED,
    //         VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    //         VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
    //         VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
    //         VkImageSubresourceRange{ VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 1, 0, 1 });
    //
    //     VkRenderingAttachmentInfo colorAttachment{};
    //     colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    //     colorAttachment.imageView = image.getView();
    //     colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    //     colorAttachment.loadOp = loadOp;
    //     colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    //     colorAttachment.clearValue = { r, g, b, 1.0f };
    //
    //     VkRenderingAttachmentInfo depthAttachment{};
    //     depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    //     depthAttachment.imageView = m_imageDepth->getView();
    //     depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    //     depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    //     depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    //     depthAttachment.clearValue.depthStencil = { 1.0f, 0 };
    //
    //     VkRenderingInfo renderingInfo{};
    //     renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    //     renderingInfo.renderArea = { {0, 0}, {Image::getWidth(image), Image::getHeight(image)} };
    //     renderingInfo.layerCount = 1;
    //     renderingInfo.colorAttachmentCount = 1;
    //     renderingInfo.pColorAttachments = &colorAttachment;
    //     renderingInfo.pDepthAttachment = &depthAttachment;
    //     renderingInfo.pStencilAttachment = &depthAttachment;
    //
    //     vkCmdBeginRendering(m_frameDatas[m_currentFrame].screenCommandBuffer, &renderingInfo);
    //
    //     updateStateCommandBuffer(m_frameDatas[m_currentFrame].screenCommandBuffer);
    // }

    // void VulkanContext::endDrawToImage(const Image& image) {
    //     vkCmdEndRendering(m_frameDatas[m_currentFrame].screenCommandBuffer);
    //
    //     tools::insertImageMemoryBarrier(m_frameDatas[m_currentFrame].screenCommandBuffer,
    //         image.getImage(),
    //         VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
    //         VK_ACCESS_SHADER_READ_BIT,
    //         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    //         VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    //         VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    //         VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
    //         VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });
    //
    //     CHECK_VK(vkEndCommandBuffer(m_frameDatas[m_currentFrame].screenCommandBuffer));
    //
    //     VkSubmitInfo submitInfo{};
    //     submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    //
    //     const std::array waitSemaphores = { m_presentSemaphore };
    //     constexpr VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    //
    //     submitInfo.waitSemaphoreCount = waitSemaphores.size();
    //     submitInfo.pWaitSemaphores = waitSemaphores.data();
    //     submitInfo.pWaitDstStageMask = waitStages;
    //     submitInfo.commandBufferCount = 1;
    //     submitInfo.pCommandBuffers = &m_frameDatas[m_currentFrame].screenCommandBuffer;
    //
    //     const std::array signalSemaphores = { m_renderSemaphore };
    //     submitInfo.signalSemaphoreCount = signalSemaphores.size();
    //     submitInfo.pSignalSemaphores = signalSemaphores.data();
    //
    //     vkQueueSubmit(m_device.getGraphis(), 1, &submitInfo, m_renderFence);
    // }

    void VulkanContext::beginScreenDraw(float r, float g, float b, VkAttachmentLoadOp loadOp) {
        CHECK_VK(vkAcquireNextImageKHR(m_device, *m_swapchain, UINT64_MAX, m_frameDatas[m_currentFrame].presentSemaphore, VK_NULL_HANDLE, &m_currentImage));

        beginDraw(m_frameDatas[m_currentFrame].screenCommandBuffer, glm::vec4(r, g, b, 1.0), loadOp, RenderTargetType::SCREEN);

        updateStateCommandBuffer(m_frameDatas[m_currentFrame].screenCommandBuffer);
    }

    void VulkanContext::endScreenDraw() {
        endDraw(m_frameDatas[m_currentFrame].screenCommandBuffer, RenderTargetType::SCREEN);
        updateStateCommandBuffer(VK_NULL_HANDLE);
    }

    void VulkanContext::beginGuiDraw(VkAttachmentLoadOp loadOp) {
        beginDraw(m_frameDatas[m_currentFrame].guiCommandBuffer, glm::vec4(0), loadOp, RenderTargetType::UI);

        updateStateCommandBuffer(m_frameDatas[m_currentFrame].guiCommandBuffer);
    }

    void VulkanContext::endGuiDraw() {
        endDraw(m_frameDatas[m_currentFrame].guiCommandBuffer, RenderTargetType::UI);
        updateStateCommandBuffer(VK_NULL_HANDLE);
    }

    void VulkanContext::draw() {
        constexpr VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &m_frameDatas[m_currentFrame].presentSemaphore;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_frameDatas[m_currentFrame].screenCommandBuffer;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &m_frameDatas[m_currentFrame].uiRenderSemaphore;

        vkQueueSubmit(m_device.getGraphis(), 1, &submitInfo, m_frameDatas[m_currentFrame].renderFence);

        CHECK_VK(vkWaitForFences(m_device, 1, &m_frameDatas[m_currentFrame].renderFence, VK_TRUE, UINT64_MAX));
        CHECK_VK(vkResetFences(m_device, 1, &m_frameDatas[m_currentFrame].renderFence));

        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &m_frameDatas[m_currentFrame].uiRenderSemaphore;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_frameDatas[m_currentFrame].guiCommandBuffer;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &m_frameDatas[m_currentFrame].renderSemaphore;

        vkQueueSubmit(m_device.getGraphis(), 1, &submitInfo, m_frameDatas[m_currentFrame].renderFence);

        CHECK_VK(vkWaitForFences(m_device, 1, &m_frameDatas[m_currentFrame].renderFence, VK_TRUE, UINT64_MAX));
        CHECK_VK(vkResetFences(m_device, 1, &m_frameDatas[m_currentFrame].renderFence));

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &m_frameDatas[m_currentFrame].renderSemaphore;

        const VkSwapchainKHR swapchains[] = { *m_swapchain };

        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapchains;
        presentInfo.pImageIndices = &m_currentImage;

        CHECK_VK(vkQueuePresentKHR(m_device.getPresent(), &presentInfo));

        m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    VulkanContext &VulkanContext::get() {
        static VulkanContext context;
        return context;
    }

    void VulkanContext::initialize() {
        auto &context = get();
        vulkanEnabled = true;
        context.initDescriptorPool();
        context.initDepth();
        context.initUploadContext();
        context.initFrameDatas();
        context.initUniformBuffers();
    }

    void VulkanContext::waitIdle() {
        auto &device = get().getDevice();
        device.waitIdle();
    }

    void VulkanContext::finalize() {
        auto &context = get();
        context.destroy();
    }

    bool VulkanContext::isVulkanEnabled() {
        return vulkanEnabled;
    }

    void VulkanContext::beginDraw(VkCommandBuffer commandBuffer, glm::vec4 clearColor, VkAttachmentLoadOp loadOp, RenderTargetType renderTarget) {
        vkResetCommandBuffer(commandBuffer, 0);

        VkCommandBufferBeginInfo commandBufferBeginInfo{};
        commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        commandBufferBeginInfo.flags = 0;
        commandBufferBeginInfo.pInheritanceInfo = nullptr;

        CHECK_VK(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo));

        VkImageLayout imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        switch (renderTarget) {
            case RenderTargetType::SCREEN:
                break;
            case RenderTargetType::UI:
                imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                break;
        }

        tools::insertImageMemoryBarrier(commandBuffer,
            m_swapchain->getImages().at(m_currentImage),
            0,
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            imageLayout,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

        tools::insertImageMemoryBarrier(commandBuffer,
            m_imageDepth->getImage(),
            0,
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            VkImageSubresourceRange{ VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 1, 0, 1 });

        VkRenderingAttachmentInfo colorAttachment{};
        colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        colorAttachment.imageView = m_swapchain->getImageViews().at(m_currentImage);
        colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachment.loadOp = loadOp;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.clearValue = { clearColor.r, clearColor.g, clearColor.b, clearColor.a };

        VkRenderingAttachmentInfo depthAttachment{};
        depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        depthAttachment.imageView = m_imageDepth->getView();
        depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        depthAttachment.clearValue.depthStencil = { 1.0f, 0 };

        VkRenderingInfo renderingInfo{};
        renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        renderingInfo.renderArea = { {0, 0}, {Window::width, Window::height} };
        renderingInfo.layerCount = 1;
        renderingInfo.colorAttachmentCount = 1;
        renderingInfo.pColorAttachments = &colorAttachment;
        renderingInfo.pDepthAttachment = &depthAttachment;
        renderingInfo.pStencilAttachment = &depthAttachment;

        vkCmdBeginRendering(commandBuffer, &renderingInfo);
    }

    void VulkanContext::endDraw(VkCommandBuffer commandBuffer, RenderTargetType renderTarget) {
        vkCmdEndRendering(commandBuffer);

        VkImageLayout imageLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        switch (renderTarget) {
            case RenderTargetType::SCREEN:
                imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                break;
            case RenderTargetType::UI:
                break;
        }

        tools::insertImageMemoryBarrier(commandBuffer,
            m_swapchain->getImages().at(m_currentImage),
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            0,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            imageLayout,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

        CHECK_VK(vkEndCommandBuffer(commandBuffer));
    }
} // vulkan