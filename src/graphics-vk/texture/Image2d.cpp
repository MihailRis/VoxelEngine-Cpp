//
// Created by chelovek on 11/19/23.
//

#include "Image2d.h"

#include <cstring>

#include "../VulkanContext.h"
#include "../Tools.h"
#include "../VulkanDefenitions.h"
#include "../../graphics/ImageData.h"
#include "../device/Buffer.h"
#include "../device/GraphicsPipeline.h"

Image2d::Image2d(const unsigned char* data, int width, int height, VkFormat format) : Image(
    {static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1},
    format,
    VK_IMAGE_VIEW_TYPE_2D,
    VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_TILING_OPTIMAL,
    VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {

    auto &device = vulkan::VulkanContext::get().getDevice();

    VkCommandPool commandPool = device.createCommadPool();
    VkCommandBuffer commandBuffer = device.createCommandBuffer(commandPool);

    const VkDeviceSize stagingSize = width * height * (format == VK_FORMAT_R8G8B8_SRGB ? 3 : 4);
    vulkan::Buffer stagingBuffer{stagingSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT};

    // copy image data to buffer
    void *stagingData;
    stagingBuffer.mapMemory(&stagingData);

    memcpy(stagingData, data, stagingSize);

    stagingBuffer.unmapMemory();

    // copy image data from buffer to image
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    CHECK_VK(vkBeginCommandBuffer(commandBuffer, &beginInfo));

    VkImageSubresourceRange range{};
    range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    range.baseMipLevel = 0;
    range.levelCount = 1;
    range.baseArrayLayer = 0;
    range.layerCount = 1;

    tools::insertImageMemoryBarrier(commandBuffer,
        m_image,
        0,
        VK_ACCESS_TRANSFER_WRITE_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        range);

    VkBufferImageCopy imageCopy{};
    imageCopy.bufferOffset = 0;
    imageCopy.bufferRowLength = 0;
    imageCopy.bufferImageHeight = 0;
    imageCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageCopy.imageSubresource.mipLevel = 0;
    imageCopy.imageSubresource.baseArrayLayer = 0;
    imageCopy.imageSubresource.layerCount = 1;
    imageCopy.imageExtent = m_extent3D;

    vkCmdCopyBufferToImage(commandBuffer, stagingBuffer, m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,  1, &imageCopy);

    tools::insertImageMemoryBarrier(commandBuffer,
        m_image,
        VK_ACCESS_TRANSFER_WRITE_BIT,
        VK_ACCESS_SHADER_READ_BIT,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
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

    vkDestroyCommandPool(device, commandPool, nullptr);
    stagingBuffer.destroy();

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

void Image2d::bind() {
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
        case ShaderType::UI:
        case ShaderType::BACKGROUND:
            samplerWrite.dstBinding = 0;
            break;
        case ShaderType::NONE:
        case ShaderType::LINES:
        case ShaderType::SKYBOX_GEN:
        default:
            return;
    }

    auto &state = vulkan::VulkanContext::get().getCurrentState();
    vulkan::vkCmdPushDescriptorSetKhr(state.commandbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getLayout(), 1, 1, &samplerWrite);
}

void Image2d::reload(unsigned char* data) {

}

Image2d* Image2d::from(const ImageData *data) {
    VkFormat format = VK_FORMAT_UNDEFINED;

    switch (data->getFormat()) {
        case ImageFormat::rgb888: format = VK_FORMAT_R8G8B8_SRGB;
        break;
        case ImageFormat::rgba8888: format = VK_FORMAT_R8G8B8A8_SRGB;
        break;
    }

    return new Image2d(static_cast<const unsigned char*>(data->getData()), data->getWidth(), data->getHeight(), format);
}

int Image2d::getWidth() {
    return m_extent3D.width;
}

int Image2d::getHeight() {
    return m_extent3D.height;
}
