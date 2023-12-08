//
// Created by chelovek on 11/16/23.
//

#include "Device.h"

#include <iostream>
#include <set>
#include <array>

#include "Inistializers.h"
#include "Instance.h"
#include "Tools.h"
#include "VulkanDefenitions.h"

Device::Device(Instance &instance, VkSurfaceKHR surface) : m_physicalDevice(instance.getPhysicalDevice()) {
    const std::vector extensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME, // this extension is used to disable the need to create renderpasses and frame buffers
        VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME,
        VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME,
        VK_KHR_MULTIVIEW_EXTENSION_NAME,
        VK_EXT_LINE_RASTERIZATION_EXTENSION_NAME
    };
    VkPhysicalDevice physicalDevice = instance.getPhysicalDevice();
    u32 queueCount = 0;
    std::vector<const char *> layers;

    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilyPropertieses{queueCount};
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, queueFamilyPropertieses.data());

    for (u32 i = 0; i < queueCount; ++i) {
        if (queueFamilyPropertieses[i].queueFlags & VK_QUEUE_GRAPHICS_BIT && !m_graphics.hasIndex()) {
            m_graphics.setIndex(i);
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);

        if (presentSupport && !m_present.hasIndex()) {
            m_present.setIndex(i);
        }

        if (m_graphics.hasIndex() && m_present.hasIndex())
            break;
    }

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    constexpr float priority[] = { 1.0f };

    std::set uniqieQueueIndexes{m_graphics.getIndex(), m_present.getIndex()};

    for (u32 uniqieQueue : uniqieQueueIndexes) {
        VkDeviceQueueCreateInfo queueCreateInfo = initializers::deviceQueueCreateInfo(uniqieQueue, 1, priority);
        queueCreateInfos.emplace_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;
    deviceFeatures.fillModeNonSolid = VK_TRUE;
    deviceFeatures.wideLines = VK_TRUE;

    VkPhysicalDeviceMultiviewFeatures physicalDeviceMultiviewFeatures{};
    physicalDeviceMultiviewFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES;
    physicalDeviceMultiviewFeatures.multiview = VK_TRUE;

    VkPhysicalDeviceDescriptorIndexingFeatures physicalDeviceDescriptorIndexingFeatures{};
    physicalDeviceDescriptorIndexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
    physicalDeviceDescriptorIndexingFeatures.descriptorBindingUniformBufferUpdateAfterBind = VK_TRUE;
    physicalDeviceDescriptorIndexingFeatures.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
    physicalDeviceDescriptorIndexingFeatures.pNext = &physicalDeviceMultiviewFeatures;

    VkPhysicalDeviceDynamicRenderingFeatures physicalDeviceDynamicRenderingFeatures{};
    physicalDeviceDynamicRenderingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
    physicalDeviceDynamicRenderingFeatures.dynamicRendering = VK_TRUE;
    physicalDeviceDynamicRenderingFeatures.pNext = &physicalDeviceDescriptorIndexingFeatures;

    VkDeviceCreateInfo deviceCreateInfo = tools::deviceCreateInfo(extensions, layers, queueCreateInfos, &deviceFeatures);
    deviceCreateInfo.pNext = &physicalDeviceDynamicRenderingFeatures;

    CHECK_VK_FUNCTION(vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &m_device));

    VkQueue graphicsQueue;
    VkQueue presentQueue;

    vkGetDeviceQueue(m_device, m_graphics.getIndex(), 0, &graphicsQueue);
    vkGetDeviceQueue(m_device, m_present.getIndex(), 0, &presentQueue);

    m_graphics.setQueue(graphicsQueue);
    m_present.setQueue(presentQueue);
}

Device::operator VkDevice() const {
    return m_device;
}

const Queue& Device::getGraphis() const {
    return m_graphics;
}

const Queue& Device::getPresent() const {
    return m_present;
}

VkImageView Device::createImageView(
    VkImage image,
    VkFormat format,
    VkImageViewType viewType,
    VkImageAspectFlags aspectFlags,
    VkComponentMapping components,
    uint32_t levelCount,
    uint32_t layerCount,
    uint32_t baseLayer) const {

    VkImageViewCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = image;
    createInfo.viewType = viewType;
    createInfo.format = format;
    createInfo.subresourceRange.aspectMask = aspectFlags;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = levelCount;
    createInfo.subresourceRange.baseArrayLayer = baseLayer;
    createInfo.subresourceRange.layerCount = layerCount;
    createInfo.components = components;

    VkImageView imageView = VK_NULL_HANDLE;

    CHECK_VK_FUNCTION(vkCreateImageView(m_device, &createInfo, nullptr, &imageView));

    return imageView;
}

VkSampler Device::createSampler(VkFilter filter, VkSamplerAddressMode addressMode, bool anisotropic) const {
    VkSamplerCreateInfo samplerCreateInfo{};
    samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCreateInfo.magFilter = filter;
    samplerCreateInfo.minFilter = filter;
    samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerCreateInfo.addressModeU = addressMode;
    samplerCreateInfo.addressModeV = addressMode;
    samplerCreateInfo.addressModeW = addressMode;
    samplerCreateInfo.mipLodBias = 0.0f;
    samplerCreateInfo.anisotropyEnable = anisotropic;
    samplerCreateInfo.minLod = 0.0f;
    samplerCreateInfo.maxLod = 0.0f;
    samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;

    VkSampler sampler = VK_NULL_HANDLE;

    CHECK_VK_FUNCTION(vkCreateSampler(m_device, &samplerCreateInfo, nullptr, &sampler));

    return sampler;
}

VkRenderPass Device::createRenderPass(VkFormat swapchainFormat, VkFormat depthFormat) const {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.flags = 0;
    colorAttachment.format = swapchainFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentDescription depthAttachment{};
    depthAttachment.flags = 0;
    depthAttachment.format = depthFormat;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorReference = { 0, VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL };
    VkAttachmentReference depthReference = { 1, VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL };

    std::array<VkSubpassDescription, 1> subpasses{};
    subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpasses[0].colorAttachmentCount = 1;
    subpasses[0].pColorAttachments = &colorReference;
    subpasses[0].pDepthStencilAttachment = &depthReference;

    std::array<VkSubpassDependency, 2> subpassDependencies{};
    subpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    subpassDependencies[0].dstSubpass = 0;
    subpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependencies[0].srcAccessMask = 0;
    subpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    subpassDependencies[1].srcSubpass = VK_SUBPASS_EXTERNAL;
    subpassDependencies[1].dstSubpass = 0;
    subpassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    subpassDependencies[1].srcAccessMask = 0;
    subpassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    subpassDependencies[1].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    const std::array attachments = { colorAttachment, depthAttachment };

    VkRenderPassCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    createInfo.attachmentCount = attachments.size();
    createInfo.pAttachments = attachments.data();
    createInfo.subpassCount = subpasses.size();
    createInfo.pSubpasses = subpasses.data();
    createInfo.dependencyCount = subpassDependencies.size();
    createInfo.pDependencies = subpassDependencies.data();

    VkRenderPass renderPass = VK_NULL_HANDLE;
    CHECK_VK_FUNCTION(vkCreateRenderPass(m_device, &createInfo, nullptr, &renderPass));

    return renderPass;
}

VkFramebuffer Device::createFramebuffer(VkRenderPass renderPass,
    const std::vector<VkImageView> &imageViews,
    VkExtent2D extent) const {

    VkFramebufferCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    createInfo.renderPass = renderPass;
    createInfo.attachmentCount = imageViews.size();
    createInfo.pAttachments = imageViews.data();
    createInfo.width = extent.width;
    createInfo.height = extent.height;
    createInfo.layers = 1;

    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    CHECK_VK_FUNCTION(vkCreateFramebuffer(m_device, &createInfo, nullptr, &framebuffer));

    return framebuffer;
}

VkFormat Device::selectSupportedFormat(const std::vector<VkFormat>& formats,
    VkImageTiling tiling,
    VkFormatFeatureFlags features) const {
    for (const auto &format : formats) {
        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(m_physicalDevice, format, &properties);

        if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & features) == features)
            return format;

        if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & features) == features)
            return format;
    }

    throw std::runtime_error("[Vulkan] failed to find supported format!");
}

VkCommandPool Device::createCommadPool() const {
    VkCommandPoolCreateInfo commandPoolCreateInfo{};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    commandPoolCreateInfo.queueFamilyIndex = m_graphics.getIndex();

    VkCommandPool commandPool = VK_NULL_HANDLE;
    CHECK_VK_FUNCTION(vkCreateCommandPool(m_device, &commandPoolCreateInfo, nullptr, &commandPool));

    return commandPool;
}

VkCommandBuffer Device::createCommandBuffer(VkCommandPool commandPool) const {
    VkCommandBufferAllocateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    info.commandPool = commandPool;
    info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    info.commandBufferCount = 1;

    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
    CHECK_VK_FUNCTION(vkAllocateCommandBuffers(m_device, &info, &commandBuffer));

    return commandBuffer;
}

void Device::waitIdle() const {
    vkDeviceWaitIdle(m_device);
}

void Device::destroy(VkCommandPool commandPool) {
    vkDestroyCommandPool(m_device, commandPool, nullptr);
}

void Device::destroy() {
    vkDestroyDevice(m_device, nullptr);
}
