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
        VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, true, 1, 6) {

    auto &device = vulkan::VulkanContext::get().getDevice();

    VkSamplerCreateInfo samplerCreateInfo{};
    samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCreateInfo.magFilter = VK_FILTER_NEAREST;
    samplerCreateInfo.minFilter = VK_FILTER_NEAREST;

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

    CHECK_VK(vkCreateSampler(device, &samplerCreateInfo, nullptr, &m_sampler));
}

void ImageCube::bind() {
    auto &device = vulkan::VulkanContext::get().getDevice();
    const auto pipeline = vulkan::VulkanContext::get().getCurrentState().pipeline;

    if (pipeline == nullptr) return;

    auto type = pipeline->getType();

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

    auto &commandBuffer = vulkan::VulkanContext::get().getCurrentState().commandbuffer;
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
