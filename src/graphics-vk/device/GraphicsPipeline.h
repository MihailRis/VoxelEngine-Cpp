//
// Created by chelovek on 11/17/23.
//

#ifndef GRAPHICSPIPELINE_H
#define GRAPHICSPIPELINE_H

#include <memory>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

#include "Shader.h"
#include "../ShaderType.h"

class Device;

class GraphicsPipeline {
    VkPipeline m_pipeline = VK_NULL_HANDLE;
    VkPipelineLayout m_layout = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_uniformsSetLayout = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_samplerSetLayout = VK_NULL_HANDLE;
    VkDescriptorSet m_uniformSet = VK_NULL_HANDLE;
    VkDescriptorSet m_samplerSet = VK_NULL_HANDLE;
    ShaderType m_shaderType;
public:
    GraphicsPipeline(VkPipeline pipeline, VkPipelineLayout layout, VkDescriptorSetLayout uniformSetLayout, VkDescriptorSetLayout samplerSetLayout, ShaderType shaderType);
    ~GraphicsPipeline();

    operator VkPipeline() const;

    ShaderType getType() const;
    VkDescriptorSet getUniformSet() const;
    VkDescriptorSet getSamplerSet() const;

    void bind(VkCommandBuffer commandBuffer);

    void destroy();

    static std::shared_ptr<GraphicsPipeline> create(const std::vector<VkPipelineShaderStageCreateInfo> &stages, ShaderType type);
};



#endif //GRAPHICSPIPELINE_H
