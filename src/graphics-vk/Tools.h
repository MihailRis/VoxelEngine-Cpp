//
// Created by chelovek on 11/19/23.
//

#ifndef TOOLS_H
#define TOOLS_H

#include <vector>
#include <GL/gl.h>
#include <vulkan/vulkan.h>
#include <type_traits>

#include "ShaderType.h"

namespace tools {

    template<typename T, typename ...Args>
    inline constexpr bool is_same_of_types_v = (std::is_same_v<T, Args> || ...);

    inline VkResult createDebugUtilsMessenger(VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
        const VkAllocationCallbacks *pAllocator,
        VkDebugUtilsMessengerEXT *pDebugMessenger) {
        const auto fn = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));

        if (fn == nullptr)
            return VK_ERROR_EXTENSION_NOT_PRESENT;

        return fn(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }

    inline void destroyDebugUtilsMessneger(VkInstance instance,
        VkDebugUtilsMessengerEXT debugUtilsMessenger,
        const VkAllocationCallbacks *pAllocator) {
        const auto fn = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));

        if (fn == nullptr)
            return;

        fn(instance, debugUtilsMessenger, pAllocator);
    }

    inline VkDeviceCreateInfo deviceCreateInfo(const std::vector<const char *> &extensions,
        const std::vector<const char *> &layers,
        const std::vector<VkDeviceQueueCreateInfo> &queueCreateInfos,
        const VkPhysicalDeviceFeatures *enabledFeatures) {
        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.enabledExtensionCount = extensions.size();
        createInfo.ppEnabledExtensionNames = extensions.data();
        createInfo.queueCreateInfoCount = queueCreateInfos.size();
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.enabledLayerCount = layers.size();
        createInfo.ppEnabledLayerNames = layers.data();
        createInfo.pEnabledFeatures = enabledFeatures;

        return createInfo;
    }

    inline void insertImageMemoryBarrier(VkCommandBuffer commandBuffer,
        VkImage image,
        VkAccessFlags srcAccessMask,
        VkAccessFlags dstAccessMask,
        VkImageLayout oldImageLayout,
        VkImageLayout newImageLayout,
        VkPipelineStageFlags srcStageMask,
        VkPipelineStageFlags dstStageMask,
        VkImageSubresourceRange subresourceRange) {

        VkImageMemoryBarrier imageMemoryBarrier{};
        imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageMemoryBarrier.srcAccessMask = srcAccessMask;
        imageMemoryBarrier.dstAccessMask = dstAccessMask;
        imageMemoryBarrier.oldLayout = oldImageLayout;
        imageMemoryBarrier.newLayout = newImageLayout;
        imageMemoryBarrier.image = image;
        imageMemoryBarrier.subresourceRange = subresourceRange;

        vkCmdPipelineBarrier(commandBuffer,
            srcStageMask,
            dstStageMask,
            0,
            0, nullptr,
            0, nullptr,
            1, &imageMemoryBarrier);
    }

    inline VkPrimitiveTopology glTopologyToVulkan(unsigned int mode) {
        switch (mode) {
            case GL_TRIANGLES: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            case GL_LINE: return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        }

        return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    }

    inline VkPolygonMode polygoneModeByType(ShaderType type) {
        switch (type) {
            default:
            case ShaderType::NONE:
            case ShaderType::MAIN:
            case ShaderType::UI:
                return VK_POLYGON_MODE_FILL;
            case ShaderType::LINES:
                return VK_POLYGON_MODE_LINE;
        }
    }
}

#endif //TOOLS_H
