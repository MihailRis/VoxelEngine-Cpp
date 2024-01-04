//
// Created by chelovek on 11/16/23.
//

#ifndef INISTIALIZERS_H
#define INISTIALIZERS_H

#include <iostream>
#include <vulkan/vulkan.h>

#include "VulkanContext.h"
#include "../typedefs.h"

namespace initializers {

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData) {
        std::cout << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }

    inline VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoExt() {
        VkDebugUtilsMessengerCreateInfoEXT createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType =
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;

        return createInfo;
    }

    inline VkDeviceQueueCreateInfo deviceQueueCreateInfo(u32 familyIndex, u32 queueCount, const float *pPriorities) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = familyIndex;
        queueCreateInfo.queueCount = queueCount;
        queueCreateInfo.pQueuePriorities = pPriorities;

        return queueCreateInfo;
    }

    inline VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfo(VkShaderModule module, VkShaderStageFlagBits stage) {
        VkPipelineShaderStageCreateInfo stageCreateInfo{};
        stageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stageCreateInfo.stage = stage;
        stageCreateInfo.module = module;
        stageCreateInfo.pName = "main";

        return stageCreateInfo;
    }

    struct UniformBufferInfo {
        bool isDynamic = false;
        VkDescriptorBufferInfo bufferInfo = {};
    };

    inline VkWriteDescriptorSet writeUniformBufferDescriptorSet(VkDescriptorSet descriptorSet, uint32_t dstBinding, const UniformBufferInfo &bufferInfo) {
        VkWriteDescriptorSet setWrite{};
        setWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        setWrite.dstSet = descriptorSet;
        setWrite.dstBinding = dstBinding;
        setWrite.dstArrayElement = 0;
        setWrite.descriptorCount = 1;
        setWrite.descriptorType = bufferInfo.isDynamic ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        setWrite.pBufferInfo = &bufferInfo.bufferInfo;

        return setWrite;
    }
}

#endif //INISTIALIZERS_H
