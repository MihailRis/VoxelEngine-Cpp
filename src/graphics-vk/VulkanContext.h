#ifndef VULKAN_CONTEXT_H
#define VULKAN_CONTEXT_H

#include <memory>

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

    struct State {
        VkCommandBuffer commandbuffer = VK_NULL_HANDLE;
        GraphicsPipeline *pipeline = nullptr;
    };

    struct FrameData {
        VkCommandPool commandPool;
        VkCommandBuffer commandBuffer;
    };

    class UniformBuffersHolder {
        std::vector<std::unique_ptr<UniformBuffer>> m_buffers;
    public:
        enum Type {
            STATE,
            LIGHT,
            FOG,
            PROJECTION_VIEW,
        };

        UniformBuffersHolder() = default;

        void initBuffers();

        const UniformBuffer *operator[](Type index) const;

        void destroy();
    };

    class VulkanContext final : Noncopybale {
        static bool vulkanEnabled;

        Instance m_instance;
        Surface m_surface;
        Device m_device;
        Swapchain m_swapchain;
        Allocator m_allocator;
        std::unique_ptr<ImageDepth> m_imageDepth = nullptr;
        VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
        uint32_t m_currentImage = 0;
        uint32_t m_currentFrame = 0;

        UniformBuffersHolder m_uniformBuffersHolder;

        FrameData m_frameDatas[MAX_FRAMES_IN_FLIGHT]{};
        VkSemaphore m_presentSemaphore, m_renderSemaphore;
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

        void updateState(GraphicsPipeline *pipeline);
        void updateState(VkCommandBuffer commandBuffer);

        void beginDraw(float r, float g, float b);
        void endDraw();

        static VulkanContext &get();

        static void initialize();
        static void waitIdle();
        static void finalize();

        static bool isVulkanEnabled();
    };

} // vulkan

#endif //VULKAN_CONTEXT_H
