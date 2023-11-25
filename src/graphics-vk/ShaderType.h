//
// Created by chelovek on 11/22/23.
//

#ifndef SHADERTYPE_H
#define SHADERTYPE_H

#include <vulkan/vulkan_core.h>
#include <string>

enum class ShaderType {
    NONE = -1,
    MAIN,
    LINES,
    SCREEN,
    UI
};

inline ShaderType toShaderType(const std::string &name) {
    if (name == "main")
        return ShaderType::MAIN;
    if (name == "lines")
        return ShaderType::LINES;
    if (name == "screen")
        return ShaderType::SCREEN;
    if (name == "ui")
        return ShaderType::UI;

    return ShaderType::NONE;
}

inline std::vector<VkDescriptorSetLayoutBinding> getSamplerBindingsByShader(ShaderType type) {
    switch (type) {
        default:
        case ShaderType::NONE:
        case ShaderType::LINES:
            return {};
        case ShaderType::MAIN:
        case ShaderType::SCREEN:
        case ShaderType::UI: {
            VkDescriptorSetLayoutBinding samplerUniformBinding{};
            samplerUniformBinding.binding = 0;
            samplerUniformBinding.descriptorCount = 1;
            samplerUniformBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            samplerUniformBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            return {samplerUniformBinding};
        }
    }
}

inline std::vector<VkDescriptorSetLayoutBinding> getUniformSetBindingsByShader(ShaderType type) {
    std::vector<VkDescriptorSetLayoutBinding> descriptroBindings{};

    switch (type) {
        case ShaderType::NONE:
            break;
        case ShaderType::MAIN: {
            VkDescriptorSetLayoutBinding stateUniformBinding{};
            stateUniformBinding.binding = 0;
            stateUniformBinding.descriptorCount = 1;
            stateUniformBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            stateUniformBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

            VkDescriptorSetLayoutBinding lightUniformBinding{};
            lightUniformBinding.binding = 1;
            lightUniformBinding.descriptorCount = 1;
            lightUniformBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            lightUniformBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

            VkDescriptorSetLayoutBinding fogUniformBinding{};
            fogUniformBinding.binding = 2;
            fogUniformBinding.descriptorCount = 1;
            fogUniformBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            fogUniformBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

            descriptroBindings.emplace_back(stateUniformBinding);
            descriptroBindings.emplace_back(lightUniformBinding);
            descriptroBindings.emplace_back(fogUniformBinding);
        }
            break;
        case ShaderType::LINES: {
            VkDescriptorSetLayoutBinding projviewUniformBinding{};
            projviewUniformBinding.binding = 0;
            projviewUniformBinding.descriptorCount = 1;
            projviewUniformBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            projviewUniformBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

            descriptroBindings.emplace_back(projviewUniformBinding);
        }
            break;
        case ShaderType::SCREEN:
            break;
        case ShaderType::UI: {
            VkDescriptorSetLayoutBinding projviewUniformBinding{};
            projviewUniformBinding.binding = 0;
            projviewUniformBinding.descriptorCount = 1;
            projviewUniformBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            projviewUniformBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

            descriptroBindings.emplace_back(projviewUniformBinding);
        }
            break;
    }

    return descriptroBindings;
}

#endif //SHADERTYPE_H
