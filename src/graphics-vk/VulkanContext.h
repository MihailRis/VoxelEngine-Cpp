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

class ImageCube;
class ImageDepth;
class GraphicsPipeline;

namespace vulkan {
    constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

    enum class RenderTargetType {
        SCREEN,
        UI,
        IMMEDIATE
    };

    struct State {
        VkCommandBuffer prevCommandbuffer = VK_NULL_HANDLE;
        VkCommandBuffer commandbuffer = VK_NULL_HANDLE;
        GraphicsPipeline *pipeline = nullptr;
        VkExtent2D viewport;
    };

    struct FrameData {
        VkCommandPool commandPool;
        VkCommandBuffer screenCommandBuffer, guiCommandBuffer, skyboxCommandBuffer, immediateCommandBuffer;

        bool skyboxRendered = false;

        VkSemaphore presentSemaphore, renderSemaphore, uiRenderSemaphore;
        VkFence renderFence;
    };

    class UniformBuffersHolder {
        std::vector<std::unique_ptr<UniformBuffer>> m_buffers;
    public:
        enum Type : size_t {
            STATE = 0,
            FOG,
            PROJECTION_VIEW,
            BACKGROUND,
            SKYBOX,
            APPLY
        };

        UniformBuffersHolder() = default;

        void initBuffers();

        UniformBuffer *operator[](Type index) const;

        void destroy();
    };

    struct UploadContext {
        VkFence uploadFence;
        VkCommandPool commandPool;
        VkCommandBuffer commandBuffer;
    };

    extern PFN_vkCmdPushDescriptorSetKHR vkCmdPushDescriptorSetKhr;

    class VulkanContext final : Noncopybale {
        static bool vulkanEnabled;

        Instance m_instance;
        Surface m_surface;
        Device m_device;
        std::unique_ptr<Swapchain> m_swapchain;
        Allocator m_allocator;
        std::unique_ptr<ImageDepth> m_imageDepth;
        VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
        uint32_t m_currentImage = 0;
        uint32_t m_currentFrame = 0;

        UploadContext m_uploadContext;

        UniformBuffersHolder m_uniformBuffersHolder;

        FrameData m_frameDatas[MAX_FRAMES_IN_FLIGHT]{};

        State m_state;

        void recreateSwapChain();
        void recreateImageDepth();
    public:
        VulkanContext();
        ~VulkanContext() = default;

        void initDescriptorPool();
        void initDepth();
        void initUploadContext();
        void initFrameDatas();
        void initUniformBuffers();
        void nextImage();

        void destroy();

        const Device &getDevice() const;
        const Allocator &getAllocator() const;

        const State &getCurrentState() const;

        const Swapchain &getSwapchain() const;
        const ImageDepth &getDepth() const;
        VkDescriptorPool getDescriptorPool() const;

        void immediateSubmit(std::function<void(VkCommandBuffer)> &&function) const;

        UniformBuffer* getUniformBuffer(UniformBuffersHolder::Type type);

        void resize();

        void updateState(GraphicsPipeline *pipeline);
        void updateStateCommandBuffer(VkCommandBuffer commandBuffer);

        VkCommandBuffer immediateBeginDraw(float r, float g, float b, VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR);
        void immediateEndDraw(VkCommandBuffer commandBuffer);

        VkCommandBuffer beginDrawSkybox(const ImageCube &image, float r, float g, float b, VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR);
        void endDrawSkybox(const ImageCube &image, VkCommandBuffer commandBuffer);

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
        void beginDraw(VkCommandBuffer commandBuffer, glm::vec4 clearColor, VkAttachmentLoadOp loadOp, RenderTargetType renderTarget, VkExtent2D renderArea);
        void endDraw(VkCommandBuffer commandBuffer, RenderTargetType renderTarget);
    };

} // vulkan

#endif //VULKAN_CONTEXT_H
