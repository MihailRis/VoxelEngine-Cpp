//
// Created by chelovek on 11/28/23.
//

#include "ImageCube.h"

#include "../VulkanContext.h"
#include "../VulkanDefenitions.h"
#include "../device/GraphicsPipeline.h"

ImageCube::ImageCube(int width, int height, VkFormat format)
    : Image({static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1},
        format,
        VK_IMAGE_VIEW_TYPE_CUBE,
        VK_IMAGE_ASPECT_COLOR_BIT,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, true, 1, 6) {

    constexpr uint32_t CUBE_LAYER_COUNT = 6;

    auto &device = vulkan::VulkanContext::get().getDevice();

    for (uint32_t i = 0; i < CUBE_LAYER_COUNT; ++i) {
        auto imageView = device.createImageView(m_image, getFormat(), VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT,
            VkComponentMapping{
                VK_COMPONENT_SWIZZLE_R,
                VK_COMPONENT_SWIZZLE_G,
                VK_COMPONENT_SWIZZLE_B,
                VK_COMPONENT_SWIZZLE_A
            },
            1,
            1, i
        );

        m_views.emplace_back(imageView);
    }

    VkCommandPool commandPool = device.createCommadPool();
    VkCommandBuffer commandBuffer = device.createCommandBuffer(commandPool);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    CHECK_VK_FUNCTION(vkBeginCommandBuffer(commandBuffer, &beginInfo));

    VkImageSubresourceRange range{};
    range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    range.baseMipLevel = 0;
    range.levelCount = 1;
    range.baseArrayLayer = 0;
    range.layerCount = 6;

    tools::insertImageMemoryBarrier(commandBuffer,
        m_image,
        0,
        VK_ACCESS_SHADER_READ_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        range);

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 0;
    submitInfo.pWaitSemaphores = nullptr;
    submitInfo.pWaitDstStageMask = nullptr;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    submitInfo.signalSemaphoreCount = 0;
    submitInfo.pSignalSemaphores = nullptr;

    vkQueueSubmit(device.getGraphis(), 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(device.getGraphis());

    VkSamplerCreateInfo samplerCreateInfo{};
    samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
    samplerCreateInfo.minFilter = VK_FILTER_LINEAR;

    samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    samplerCreateInfo.anisotropyEnable = VK_TRUE;
    samplerCreateInfo.maxAnisotropy = 1.0f;
    samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_WHITE;
    samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
    samplerCreateInfo.compareEnable = VK_FALSE;
    samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerCreateInfo.mipLodBias = 0.0f;
    samplerCreateInfo.minLod = 0.0f;
    samplerCreateInfo.maxLod = 0.0f;

    CHECK_VK_FUNCTION(vkCreateSampler(device, &samplerCreateInfo, nullptr, &m_sampler));

    vkDestroyCommandPool(device, commandPool, nullptr);
}

ImageCube::~ImageCube() {
    auto &device = vulkan::VulkanContext::get().getDevice();
    for (const auto &view: m_views) {
        vkDestroyImageView(device, view, nullptr);
    }

    Image::~Image();
}

void ImageCube::bind() {
    const auto &state = vulkan::VulkanContext::get().getCurrentState();

    if (state.pipeline == nullptr) return;

    const auto type = state.pipeline->getType();

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = m_imageView;
    imageInfo.sampler = m_sampler;

    VkWriteDescriptorSet samplerWrite{};
    samplerWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    samplerWrite.dstArrayElement = 0;
    samplerWrite.descriptorCount = 1;
    samplerWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerWrite.pImageInfo = &imageInfo;

    switch (type) {
        case ShaderType::MAIN:
            samplerWrite.dstBinding = 1;
            break;
        case ShaderType::BACKGROUND:
            samplerWrite.dstBinding = 0;
            break;
        case ShaderType::UI:
        case ShaderType::NONE:
        case ShaderType::LINES:
        case ShaderType::SKYBOX_GEN:
        default:
            return;
    }

    if (state.commandBuffer == VK_NULL_HANDLE) return;
    vulkan::vkCmdPushDescriptorSetKhr(state.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, state.pipeline->getLayout(), 1, 1, &samplerWrite);
}

void ImageCube::bind(VkCommandBuffer commandBuffer, const GraphicsPipeline *pipeline) {
    if (pipeline == nullptr) return;

    const auto type = pipeline->getType();

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = m_imageView;
    imageInfo.sampler = m_sampler;

    VkWriteDescriptorSet samplerWrite{};
    samplerWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    samplerWrite.dstArrayElement = 0;
    samplerWrite.descriptorCount = 1;
    samplerWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerWrite.pImageInfo = &imageInfo;

    switch (type) {
        case ShaderType::MAIN:
            samplerWrite.dstBinding = 1;
        break;
        case ShaderType::BACKGROUND:
            samplerWrite.dstBinding = 0;
        break;
        case ShaderType::UI:
        case ShaderType::NONE:
        case ShaderType::LINES:
        case ShaderType::SKYBOX_GEN:
        default:
            return;
    }

    if (commandBuffer == VK_NULL_HANDLE) return;
    vulkan::vkCmdPushDescriptorSetKhr(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getLayout(), 1, 1, &samplerWrite);
}

void ImageCube::reload(unsigned char* data) {
}

int ImageCube::getWidth() const {
    return static_cast<int>(m_extent3D.width);
}

int ImageCube::getHeight() const {
    return static_cast<int>(m_extent3D.height);
}
