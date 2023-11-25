//
// Created by chelovek on 11/17/23.
//

#include "GraphicsPipeline.h"

#include <iostream>
#include <array>

#include "../Device.h"
#include "../Inistializers.h"
#include "../VulkanContext.h"
#include "../VulkanDefenitions.h"
#include "../Vertices.h"

// TODO: to much dynamic states...
constexpr VkDynamicState DYNAMIC_STATES[] = {
    VK_DYNAMIC_STATE_LINE_WIDTH,
    VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY,
    // VK_DYNAMIC_STATE_VIEWPORT
};

GraphicsPipeline::GraphicsPipeline(VkPipeline pipeline, VkPipelineLayout layout, VkDescriptorSetLayout uniformSetLayout, VkDescriptorSetLayout samplerSetLayout, ShaderType shaderType)
    : m_pipeline(pipeline),
      m_layout(layout),
      m_uniformsSetLayout(uniformSetLayout),
      m_samplerSetLayout(samplerSetLayout),
      m_shaderType(shaderType) {
    const auto &context = vulkan::VulkanContext::get();
    auto &device= context.getDevice();

    VkDescriptorPool descriptorPool = context.getDescriptorPool();

    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
    descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocateInfo.descriptorPool = descriptorPool;
    descriptorSetAllocateInfo.descriptorSetCount = 1;
    descriptorSetAllocateInfo.pSetLayouts = &m_uniformsSetLayout;

    CHECK_VK(vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, &m_uniformSet));

    descriptorSetAllocateInfo.pSetLayouts = &m_samplerSetLayout;
    CHECK_VK(vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, &m_samplerSet));

    const auto stateBufferInfo = context.getUniformBuffer(vulkan::UniformBuffersHolder::STATE)->getBufferInfo();
    const auto lightBufferInfo = context.getUniformBuffer(vulkan::UniformBuffersHolder::LIGHT)->getBufferInfo();
    const auto fogBufferInfo = context.getUniformBuffer(vulkan::UniformBuffersHolder::FOG)->getBufferInfo();
    const auto projectionViewBufferInfo = context.getUniformBuffer(vulkan::UniformBuffersHolder::PROJECTION_VIEW)->getBufferInfo();

    switch (shaderType) {
        case ShaderType::MAIN: {
            const std::array writeSets = {
                initializers::writeUniformBufferDescriptorSet(m_uniformSet, 0, &stateBufferInfo),
                initializers::writeUniformBufferDescriptorSet(m_uniformSet, 1, &lightBufferInfo),
                initializers::writeUniformBufferDescriptorSet(m_uniformSet, 2, &fogBufferInfo),
            };

            vkUpdateDescriptorSets(device, writeSets.size(), writeSets.data(), 0, nullptr);
        }
            break;
        case ShaderType::LINES: {
            const std::array writeSets = {
                initializers::writeUniformBufferDescriptorSet(m_uniformSet, 0, &projectionViewBufferInfo),
            };

            vkUpdateDescriptorSets(device, writeSets.size(), writeSets.data(), 0, nullptr);
        }
            break;
        case ShaderType::UI: {
            const std::array writeSets = {
                initializers::writeUniformBufferDescriptorSet(m_uniformSet, 0, &projectionViewBufferInfo),
            };

            vkUpdateDescriptorSets(device, writeSets.size(), writeSets.data(), 0, nullptr);
        }
            break;
        case ShaderType::NONE:
        case ShaderType::SCREEN:
        default:
            break;
    }
}

GraphicsPipeline::~GraphicsPipeline() {
    destroy();
}

GraphicsPipeline::operator VkPipeline() const {
    return m_pipeline;
}

ShaderType GraphicsPipeline::getType() const {
    return m_shaderType;
}

VkDescriptorSet GraphicsPipeline::getUniformSet() const {
    return m_uniformSet;
}

VkDescriptorSet GraphicsPipeline::getSamplerSet() const {
    return m_samplerSet;
}

void GraphicsPipeline::bind(VkCommandBuffer commandBuffer) {
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
    const VkDescriptorSet sets[] = { m_uniformSet, m_samplerSet };
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_layout, 0, 2, sets, 0, nullptr);
}

void GraphicsPipeline::destroy() {
    auto &device = vulkan::VulkanContext::get().getDevice();
    vkDestroyPipeline(device, m_pipeline, nullptr);
    vkDestroyPipelineLayout(device, m_layout, nullptr);
    vkDestroyDescriptorSetLayout(device, m_uniformsSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, m_samplerSetLayout, nullptr);
}

std::shared_ptr<GraphicsPipeline> GraphicsPipeline::create(const std::vector<VkPipelineShaderStageCreateInfo>& stages, ShaderType type) {
    constexpr size_t dynamicStateCount = sizeof(DYNAMIC_STATES) / sizeof(VkDynamicState);

    VkVertexInputBindingDescription binding = getVertexBindingByType(type);
    std::vector<VkDescriptorSetLayoutBinding> uniformBindings = getUniformSetBindingsByShader(type);
    std::vector<VkDescriptorSetLayoutBinding> samplerBindings = getSamplerBindingsByShader(type);
    std::vector<VkVertexInputAttributeDescription> attributes = getVertexAttributeDescriptionByType(type);

    std::vector<VkDescriptorBindingFlags> descriptorBindingFlags;

    switch (type)
    {
        case ShaderType::NONE:
            throw std::runtime_error("Failed to initialize pipeline");
        case ShaderType::MAIN:
        case ShaderType::SCREEN:
        case ShaderType::UI:
            descriptorBindingFlags.emplace_back(VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT);
        break;
        case ShaderType::LINES:
        default:
            break;
    }

    auto &device = vulkan::VulkanContext::get().getDevice();

    VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsCreateInfo{};
    bindingFlagsCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
    bindingFlagsCreateInfo.bindingCount = descriptorBindingFlags.size();
    bindingFlagsCreateInfo.pBindingFlags = descriptorBindingFlags.data();

    VkDescriptorSetLayoutCreateInfo uniformsSetLayoutCreateInfo{};
    uniformsSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    uniformsSetLayoutCreateInfo.bindingCount = uniformBindings.size();
    uniformsSetLayoutCreateInfo.pBindings = uniformBindings.data();

    VkDescriptorSetLayoutCreateInfo samplerSetLayoutCreateInfo{};
    samplerSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    samplerSetLayoutCreateInfo.bindingCount = samplerBindings.size();
    samplerSetLayoutCreateInfo.pBindings = samplerBindings.data();
    samplerSetLayoutCreateInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
    samplerSetLayoutCreateInfo.pNext = &bindingFlagsCreateInfo;

    VkDescriptorSetLayout uniformSetLayout = VK_NULL_HANDLE;
    CHECK_VK(vkCreateDescriptorSetLayout(device, &uniformsSetLayoutCreateInfo, nullptr, &uniformSetLayout));

    VkDescriptorSetLayout samplerSetLayout = VK_NULL_HANDLE;
    CHECK_VK(vkCreateDescriptorSetLayout(device, &samplerSetLayoutCreateInfo, nullptr, &samplerSetLayout));

    std::array setLayouts = { uniformSetLayout, samplerSetLayout };

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = setLayouts.size();
    pipelineLayoutCreateInfo.pSetLayouts = setLayouts.data();

    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    CHECK_VK(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout));

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = dynamicStateCount;
    dynamicState.pDynamicStates = DYNAMIC_STATES;

    VkPipelineVertexInputStateCreateInfo vertexInput{};
    vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInput.vertexAttributeDescriptionCount = attributes.size();
    vertexInput.pVertexAttributeDescriptions = attributes.data();
    vertexInput.vertexBindingDescriptionCount = 1;
    vertexInput.pVertexBindingDescriptions = &binding;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    const VkExtent2D extent2D = vulkan::VulkanContext::get().getSwapchain().getExtent();

    VkViewport viewport{};
    viewport.x = 0.f;
    viewport.y = 0.f;
    viewport.width = extent2D.width;
    viewport.height = extent2D.height;
    viewport.minDepth = 0.f;
    viewport.maxDepth = 1.f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = extent2D;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    VkFormat sawpchainFormat = vulkan::VulkanContext::get().getSwapchain().getFormat();
    auto &depth = vulkan::VulkanContext::get().getDepth();
    VkPipelineRenderingCreateInfo renderingCreateInfo{};
    renderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    renderingCreateInfo.colorAttachmentCount = 1;
    renderingCreateInfo.pColorAttachmentFormats = &sawpchainFormat;
    renderingCreateInfo.depthAttachmentFormat = depth.getFormat();
    renderingCreateInfo.stencilAttachmentFormat = depth.getFormat();

    VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo{};
    graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    graphicsPipelineCreateInfo.pDynamicState = &dynamicState;
    graphicsPipelineCreateInfo.stageCount = stages.size();
    graphicsPipelineCreateInfo.pStages = stages.data();
    graphicsPipelineCreateInfo.pVertexInputState = &vertexInput;
    graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssembly;
    graphicsPipelineCreateInfo.pViewportState = &viewportState;
    graphicsPipelineCreateInfo.pRasterizationState = &rasterizer;
    graphicsPipelineCreateInfo.pMultisampleState = &multisampling;
    graphicsPipelineCreateInfo.pDepthStencilState = &depthStencil;
    graphicsPipelineCreateInfo.pColorBlendState = &colorBlending;
    graphicsPipelineCreateInfo.layout = pipelineLayout;
    graphicsPipelineCreateInfo.pNext = &renderingCreateInfo;

    VkPipeline pipeline = VK_NULL_HANDLE;
    CHECK_VK(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, nullptr, &pipeline));

    return std::make_shared<GraphicsPipeline>(pipeline, pipelineLayout, uniformSetLayout, samplerSetLayout, type);
}
