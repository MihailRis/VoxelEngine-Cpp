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
    UI,
    BACKGROUND,
    SKYBOX_GEN
};

inline ShaderType toShaderType(const std::string &name) {
    if (name == "main")
        return ShaderType::MAIN;
    if (name == "lines")
        return ShaderType::LINES;
    if (name == "ui")
        return ShaderType::UI;
    if (name == "background")
        return ShaderType::BACKGROUND;
    if (name == "skybox_gen")
        return ShaderType::SKYBOX_GEN;

    return ShaderType::NONE;
}

inline std::vector<VkDescriptorSetLayoutBinding> getSamplerBindingsByShader(ShaderType type) {
    switch (type) {
        default:
        case ShaderType::NONE:
        case ShaderType::LINES:
            return {};
        case ShaderType::MAIN: {
            VkDescriptorSetLayoutBinding samplerUniformBinding{};
            samplerUniformBinding.binding = 0;
            samplerUniformBinding.descriptorCount = 1;
            samplerUniformBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            samplerUniformBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

            VkDescriptorSetLayoutBinding samplerCubeUniformBinding{};
            samplerCubeUniformBinding.binding = 1;
            samplerCubeUniformBinding.descriptorCount = 1;
            samplerCubeUniformBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            samplerCubeUniformBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
            return {samplerUniformBinding, samplerCubeUniformBinding};
        }
        case ShaderType::UI:
        case ShaderType::BACKGROUND: {
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
        default:
        case ShaderType::NONE:
            break;
        case ShaderType::MAIN: {
            VkDescriptorSetLayoutBinding stateUniformBinding{};
            stateUniformBinding.binding = 0;
            stateUniformBinding.descriptorCount = 1;
            stateUniformBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            stateUniformBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

            // VkDescriptorSetLayoutBinding lightUniformBinding{};
            // lightUniformBinding.binding = 1;
            // lightUniformBinding.descriptorCount = 1;
            // lightUniformBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            // lightUniformBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

            VkDescriptorSetLayoutBinding fogUniformBinding{};
            fogUniformBinding.binding = 1;
            fogUniformBinding.descriptorCount = 1;
            fogUniformBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            fogUniformBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

            descriptroBindings.emplace_back(stateUniformBinding);
            descriptroBindings.emplace_back(fogUniformBinding);
        } break;
        case ShaderType::LINES: {
            VkDescriptorSetLayoutBinding projviewUniformBinding{};
            projviewUniformBinding.binding = 0;
            projviewUniformBinding.descriptorCount = 1;
            projviewUniformBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            projviewUniformBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

            descriptroBindings.emplace_back(projviewUniformBinding);
        } break;
        case ShaderType::UI: {
            VkDescriptorSetLayoutBinding projviewUniformBinding{};
            projviewUniformBinding.binding = 0;
            projviewUniformBinding.descriptorCount = 1;
            projviewUniformBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            projviewUniformBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

            descriptroBindings.emplace_back(projviewUniformBinding);
        } break;
        case ShaderType::BACKGROUND: {
            VkDescriptorSetLayoutBinding backgroundUniformBinding{};
            backgroundUniformBinding.binding = 0;
            backgroundUniformBinding.descriptorCount = 1;
            backgroundUniformBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            backgroundUniformBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

            descriptroBindings.emplace_back(backgroundUniformBinding);
        } break;
        case ShaderType::SKYBOX_GEN: {
            VkDescriptorSetLayoutBinding skyboxUniformBinding{};
            skyboxUniformBinding.binding = 0;
            skyboxUniformBinding.descriptorCount = 1;
            skyboxUniformBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            skyboxUniformBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

            descriptroBindings.emplace_back(skyboxUniformBinding);
        } break;
    }

    return descriptroBindings;
}

#endif //SHADERTYPE_H
