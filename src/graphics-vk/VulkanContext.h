#ifndef VULKAN_CONTEXT_H
#define VULKAN_CONTEXT_H

#include <functional>
#include <memory>
#include <glm/vec4.hpp>

#include "Allocator.h"
#include "Device.h"
#include "Instance.h"
#include "Surface.h"
#include "device/Swapchain.h"
#include "../util/Noncopybale.h"
#include "device/UniformBuffer.h"
#include "texture/ImageDepth.h"

class GraphicsPipeline;

namespace vulkan {
    constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

    enum class RenderTargetType {
        SCREEN,
        UI
    };

    struct State {
        VkCommandBuffer commandbuffer = VK_NULL_HANDLE;
        GraphicsPipeline *pipeline = nullptr;
    };

    struct FrameData {
        VkCommandPool commandPool;
        VkCommandBuffer screenCommandBuffer, guiCommandBuffer;
    };

    class UniformBuffersHolder {
        std::vector<std::unique_ptr<UniformBuffer>> m_buffers;
    public:
        enum Type : size_t {
            STATE = 0,
            FOG,
            PROJECTION_VIEW,
            BACKGROUND,
            SKYBOX
        };

        UniformBuffersHolder() = default;

        void initBuffers();

        const UniformBuffer *operator[](Type index) const;

        void destroy();
    };

    extern PFN_vkCmdPushDescriptorSetKHR vkCmdPushDescriptorSetKhr;

    class VulkanContext final : Noncopybale {
        static bool vulkanEnabled;

        Instance m_instance;
        Surface m_surface;
        Device m_device;
        std::unique_ptr<Swapchain> m_swapchain;
        Allocator m_allocator;
        std::unique_ptr<ImageDepth> m_imageDepth = nullptr;
        VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
        uint32_t m_currentImage = 0;
        uint32_t m_currentFrame = 0;

        UniformBuffersHolder m_uniformBuffersHolder;

        FrameData m_frameDatas[MAX_FRAMES_IN_FLIGHT]{};
        VkSemaphore m_presentSemaphore, m_renderSemaphore, m_uiRenderSemaphore;
        VkFence m_renderFence;

        State m_state;
    public:
        VulkanContext();
        ~VulkanContext() = default;

        void initDescriptorPool();
        void initDepth();
        void initFrameDatas();
        void initUniformBuffers();

        void destroy();

        const Device &getDevice() const;
        const Allocator &getAllocator() const;

        const State &getCurrentState() const;

        const Swapchain &getSwapchain() const;
        const ImageDepth &getDepth() const;
        VkDescriptorPool getDescriptorPool() const;
        const UniformBuffer *getUniformBuffer(UniformBuffersHolder::Type type) const;

        void recreateSwapChain();

        void updateState(GraphicsPipeline *pipeline);
        void updateStateCommandBuffer(VkCommandBuffer commandBuffer);

        void beginDrawToImage(const Image &image, float r, float g, float b, VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR);
        void endDrawToImage(const Image &image);

        void beginScreenDraw(float r, float g, float b, VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR);
        void endScreenDraw();

        void beginGuiDraw(VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_LOAD);
        void endGuiDraw();

        void draw();

        static VulkanContext &get();

        static void initialize();
        static void waitIdle();
        static void finalize();

        static bool isVulkanEnabled();

    private:
        void beginDraw(VkCommandBuffer commandBuffer, glm::vec4 clearColor, VkAttachmentLoadOp loadOp, RenderTargetType renderTarget);
        void endDraw(VkCommandBuffer commandBuffer, RenderTargetType renderTarget);
    };

} // vulkan

#endif //VULKAN_CONTEXT_H
