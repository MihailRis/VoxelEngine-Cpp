#include "VulkanContext.h"

#include <iostream>
#include <array>

#include "Tools.h"
#include "VulkanDefenitions.h"
#include "../window/Window.h"
#include "device/GraphicsPipeline.h"
#include "texture/ImageDepth.h"
#include "texture/ImageCube.h"
#include "uniforms/ApplyUniform.h"
#include "uniforms/BackgroundUniform.h"
#include "uniforms/FogUniform.h"
#include "uniforms/ProjectionViewUniform.h"
#include "uniforms/SkyboxUniform.h"
#include "uniforms/StateUniform.h"

constexpr uint32_t DESCRIPTOR_SET_COUNT = 1024;

namespace vulkan {

    PFN_vkCmdPushDescriptorSetKHR vkCmdPushDescriptorSetKhr = nullptr;

    bool VulkanContext::vulkanEnabled = false;

    void VulkanContext::recreateSwapChain() {
        m_swapchain->destroy();
        m_swapchain.reset();
        m_swapchain = std::make_unique<Swapchain>(m_surface, m_device);
    }

    void VulkanContext::recreateImageDepth() {
        m_imageDepth->destroy();
        m_imageDepth.reset();
        initDepth();
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
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, DESCRIPTOR_SET_COUNT },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, DESCRIPTOR_SET_COUNT }
        };

        VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{};
        descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptorPoolCreateInfo.maxSets = DESCRIPTOR_SET_COUNT;
        descriptorPoolCreateInfo.poolSizeCount = descriptorPoolSizes.size();
        descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizes.data();
        descriptorPoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;

        CHECK_VK_FUNCTION(vkCreateDescriptorPool(m_device, &descriptorPoolCreateInfo, nullptr, &m_descriptorPool));
    }

    void VulkanContext::initDepth() {
        const VkExtent2D swapChainExtent = m_swapchain->getExtent();
        m_imageDepth = std::make_unique<ImageDepth>(VkExtent3D{swapChainExtent.width, swapChainExtent.height, 1});
    }

    void VulkanContext::initUploadContext() {
        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

        VkCommandPoolCreateInfo commandPoolCreateInfo{};
        commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        commandPoolCreateInfo.queueFamilyIndex = m_device.getGraphis().getIndex();

        VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
        commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferAllocateInfo.commandBufferCount = 1;

        CHECK_VK_FUNCTION(vkCreateCommandPool(m_device, &commandPoolCreateInfo, nullptr, &m_uploadContext.commandPool));

        commandBufferAllocateInfo.commandPool = m_uploadContext.commandPool;

        CHECK_VK_FUNCTION(vkAllocateCommandBuffers(m_device, &commandBufferAllocateInfo, &m_uploadContext.commandBuffer));

        CHECK_VK_FUNCTION(vkCreateFence(m_device, &fenceInfo, nullptr, &m_uploadContext.uploadFence));
    }

    void VulkanContext::initFrameDatas() {
        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

        VkCommandPoolCreateInfo commandPoolCreateInfo{};
        commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        commandPoolCreateInfo.queueFamilyIndex = m_device.getGraphis().getIndex();

        VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
        commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferAllocateInfo.commandBufferCount = 1;

        for (auto &m_frameData : m_frameDatas) {
            CHECK_VK_FUNCTION(vkCreateCommandPool(m_device, &commandPoolCreateInfo, nullptr, &m_frameData.commandPool));

            commandBufferAllocateInfo.commandPool = m_frameData.commandPool;

            CHECK_VK_FUNCTION(vkAllocateCommandBuffers(m_device, &commandBufferAllocateInfo, &m_frameData.screenCommandBuffer));
            CHECK_VK_FUNCTION(vkAllocateCommandBuffers(m_device, &commandBufferAllocateInfo, &m_frameData.guiCommandBuffer));
            CHECK_VK_FUNCTION(vkAllocateCommandBuffers(m_device, &commandBufferAllocateInfo, &m_frameData.skyboxCommandBuffer));
            CHECK_VK_FUNCTION(vkAllocateCommandBuffers(m_device, &commandBufferAllocateInfo, &m_frameData.immediateCommandBuffer));

            CHECK_VK_FUNCTION(vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_frameData.presentSemaphore));
            CHECK_VK_FUNCTION(vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_frameData.renderSemaphore));
            CHECK_VK_FUNCTION(vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_frameData.uiRenderSemaphore));
            CHECK_VK_FUNCTION(vkCreateFence(m_device, &fenceInfo, nullptr, &m_frameData.renderFence));
        }
    }

    void VulkanContext::nextImage() {
        CHECK_VK_FUNCTION(vkAcquireNextImageKHR(m_device, *m_swapchain, UINT64_MAX, m_frameDatas[m_currentFrame].presentSemaphore, VK_NULL_HANDLE, &m_currentImage));
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

        vkDestroyFence(m_device, m_uploadContext.uploadFence, nullptr);
        vkDestroyCommandPool(m_device, m_uploadContext.commandPool, nullptr);

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

        CHECK_VK_FUNCTION(vkBeginCommandBuffer(m_uploadContext.commandBuffer, &beginInfo));

        function(m_uploadContext.commandBuffer);

        CHECK_VK_FUNCTION(vkEndCommandBuffer(m_uploadContext.commandBuffer));

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_uploadContext.commandBuffer;

        CHECK_VK_FUNCTION(vkQueueSubmit(m_device.getGraphis(), 1, &submitInfo, m_uploadContext.uploadFence));

        vkWaitForFences(m_device, 1, &m_uploadContext.uploadFence, VK_TRUE, UINT64_MAX);
        vkResetFences(m_device, 1, &m_uploadContext.uploadFence);

        vkResetCommandPool(m_device, m_uploadContext.commandPool, 0);
    }

    void VulkanContext::resize() {
        recreateSwapChain();
        recreateImageDepth();
        nextImage();
    }

    void VulkanContext::updateState(GraphicsPipeline *pipeline) {
        m_state.pipeline = pipeline;
    }

    void VulkanContext::updateStateCommandBuffer(VkCommandBuffer commandBuffer) {
        m_state.prevCommandbuffer = m_state.commandBuffer;
        m_state.commandBuffer = commandBuffer;
    }

    VkCommandBuffer VulkanContext::immediateBeginDraw(float r, float g, float b, VkAttachmentLoadOp loadOp) {
        CHECK_VK_FUNCTION(vkResetCommandBuffer(m_frameDatas[m_currentFrame].immediateCommandBuffer, 0));

        const auto swapchainExtent = m_swapchain->getExtent();
        beginDraw(m_frameDatas[m_currentFrame].immediateCommandBuffer, glm::vec4(r, g, b, 1.0f), loadOp, RenderTargetType::IMMEDIATE, swapchainExtent);

        return m_frameDatas[m_currentFrame].immediateCommandBuffer;
    }

    void VulkanContext::immediateEndDraw(VkCommandBuffer commandBuffer) {
        endDraw(commandBuffer, RenderTargetType::IMMEDIATE);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        CHECK_VK_FUNCTION(vkQueueSubmit(m_device.getGraphis(), 1, &submitInfo, m_frameDatas[m_currentFrame].renderFence));
        CHECK_VK_FUNCTION(vkWaitForFences(m_device, 1, &m_frameDatas[m_currentFrame].renderFence, VK_TRUE, UINT64_MAX));
        CHECK_VK_FUNCTION(vkResetFences(m_device, 1, &m_frameDatas[m_currentFrame].renderFence));
    }

    VkCommandBuffer VulkanContext::beginDrawSkybox(const ImageCube& image, float r, float g, float b, VkAttachmentLoadOp loadOp) {
        VkCommandBufferBeginInfo commandBufferBeginInfo{};
        commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        CHECK_VK_FUNCTION(vkBeginCommandBuffer(m_frameDatas[m_currentFrame].skyboxCommandBuffer, &commandBufferBeginInfo));

        m_frameDatas[m_currentFrame].skyboxRendered = true;

        tools::insertImageMemoryBarrier(m_frameDatas[m_currentFrame].skyboxCommandBuffer,
            image.getImage(),
            VK_ACCESS_SHADER_READ_BIT,
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, image.getLayerCount() });

        VkRenderingAttachmentInfo colorAttachment{};
        colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        colorAttachment.imageView = image.getView();
        colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachment.loadOp = loadOp;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.clearValue = { r, g, b, 1.0f };

        VkRenderingInfo renderingInfo{};
        renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        renderingInfo.renderArea = { {0, 0}, {Image::getWidth(image), Image::getHeight(image)} };
        renderingInfo.layerCount = image.getLayerCount();
        renderingInfo.viewMask = 0b111111;
        renderingInfo.colorAttachmentCount = 1;
        renderingInfo.pColorAttachments = &colorAttachment;

        vkCmdBeginRendering(m_frameDatas[m_currentFrame].skyboxCommandBuffer, &renderingInfo);

        return  m_frameDatas[m_currentFrame].skyboxCommandBuffer;
    }

    void VulkanContext::endDrawSkybox(const ImageCube& image, VkCommandBuffer commandBuffer) {
        vkCmdEndRendering(commandBuffer);

        tools::insertImageMemoryBarrier(commandBuffer,
            image.getImage(),
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            VK_ACCESS_SHADER_READ_BIT,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, image.getLayerCount() },
            VK_DEPENDENCY_BY_REGION_BIT);

        CHECK_VK_FUNCTION(vkEndCommandBuffer(commandBuffer));

        constexpr VkPipelineStageFlags stages[] = { VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT };

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;
        submitInfo.pWaitDstStageMask = stages;

        CHECK_VK_FUNCTION(vkQueueSubmit(m_device.getGraphis(), 1, &submitInfo, m_frameDatas[m_currentFrame].renderFence));

        CHECK_VK_RESULT(vkWaitForFences(m_device, 1, &m_frameDatas[m_currentFrame].renderFence, VK_TRUE, UINT64_MAX));
        CHECK_VK_RESULT(vkResetFences(m_device, 1, &m_frameDatas[m_currentFrame].renderFence));
    }

    void VulkanContext::beginScreenDraw(float r, float g, float b, VkAttachmentLoadOp loadOp) {
        const auto swapchainExtent = m_swapchain->getExtent();
        beginDraw(m_frameDatas[m_currentFrame].screenCommandBuffer, glm::vec4(r, g, b, 1.0), loadOp, RenderTargetType::SCREEN, swapchainExtent);
        updateStateCommandBuffer(m_frameDatas[m_currentFrame].screenCommandBuffer);
        m_state.viewport = swapchainExtent;
    }

    void VulkanContext::endScreenDraw() {
        endDraw(m_frameDatas[m_currentFrame].screenCommandBuffer, RenderTargetType::SCREEN);
        updateStateCommandBuffer(VK_NULL_HANDLE);
    }

    void VulkanContext::beginGuiDraw(VkAttachmentLoadOp loadOp) {
        const VkExtent2D swapchainExtent = m_swapchain->getExtent();
        beginDraw(m_frameDatas[m_currentFrame].guiCommandBuffer, glm::vec4(0), loadOp, RenderTargetType::UI, swapchainExtent);
        updateStateCommandBuffer(m_frameDatas[m_currentFrame].guiCommandBuffer);
        m_state.viewport = swapchainExtent;
    }

    void VulkanContext::endGuiDraw() {
        endDraw(m_frameDatas[m_currentFrame].guiCommandBuffer, RenderTargetType::UI);
        updateStateCommandBuffer(VK_NULL_HANDLE);
    }

    void VulkanContext::draw() {
        constexpr VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

        const std::array commandBuffers = {
            m_frameDatas[m_currentFrame].screenCommandBuffer,
            m_frameDatas[m_currentFrame].guiCommandBuffer
        };

        const std::array signalSemaphores = {
            m_frameDatas[m_currentFrame].renderSemaphore,
            m_frameDatas[m_currentFrame].uiRenderSemaphore
        };

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &m_frameDatas[m_currentFrame].presentSemaphore;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = commandBuffers.size();
        submitInfo.pCommandBuffers = commandBuffers.data();
        submitInfo.signalSemaphoreCount = signalSemaphores.size();
        submitInfo.pSignalSemaphores = signalSemaphores.data();

        CHECK_VK_FUNCTION(vkQueueSubmit(m_device.getGraphis(), 1, &submitInfo, m_frameDatas[m_currentFrame].renderFence));

        (vkWaitForFences(m_device, 1, &m_frameDatas[m_currentFrame].renderFence, VK_TRUE, UINT64_MAX));
        (vkResetFences(m_device, 1, &m_frameDatas[m_currentFrame].renderFence));

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = signalSemaphores.size();
        presentInfo.pWaitSemaphores = signalSemaphores.data();

        const VkSwapchainKHR swapchains[] = { *m_swapchain };

        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapchains;
        presentInfo.pImageIndices = &m_currentImage;

        const VkResult presentResult = vkQueuePresentKHR(m_device.getPresent(), &presentInfo);

        m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

        if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR || Window::isResized) {
            Window::isResized = false;
            resize();
            return;
        }

        const VkResult result = vkAcquireNextImageKHR(m_device, *m_swapchain, UINT64_MAX, m_frameDatas[m_currentFrame].presentSemaphore, VK_NULL_HANDLE, &m_currentImage);
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            resize();
        }
    }

    VulkanContext &VulkanContext::get() {
        static VulkanContext context;
        return context;
    }

    void VulkanContext::initialize() {
        VulkanContext &context = get();
        vulkanEnabled = true;
        context.initDescriptorPool();
        context.initDepth();
        context.initUploadContext();
        context.initFrameDatas();
        context.nextImage();
    }

    void VulkanContext::waitIdle() {
        const Device &device = get().getDevice();
        device.waitIdle();
    }

    void VulkanContext::finalize() {
        VulkanContext &context = get();
        context.destroy();
    }

    bool VulkanContext::isVulkanEnabled() {
        return vulkanEnabled;
    }

    void VulkanContext::beginDraw(VkCommandBuffer commandBuffer, glm::vec4 clearColor, VkAttachmentLoadOp loadOp, RenderTargetType renderTarget, VkExtent2D renderArea) {
        VkCommandBufferBeginInfo commandBufferBeginInfo{};
        commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        commandBufferBeginInfo.flags = 0;
        commandBufferBeginInfo.pInheritanceInfo = nullptr;
        commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        CHECK_VK_FUNCTION(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo));

        VkImageLayout imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        switch (renderTarget) {
            case RenderTargetType::SCREEN:
            case RenderTargetType::IMMEDIATE:
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
        renderingInfo.renderArea = { {0, 0}, renderArea };
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
            case RenderTargetType::IMMEDIATE:
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

        CHECK_VK_FUNCTION(vkEndCommandBuffer(commandBuffer));
    }
} // vulkan
