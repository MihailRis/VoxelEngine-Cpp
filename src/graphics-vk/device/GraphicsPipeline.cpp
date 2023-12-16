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
#include "../../window/Window.h"
#include "../uniforms/DynamicConstants.h"
#include "../texture/ImageDepth.h"

constexpr VkDynamicState DYNAMIC_STATES[] = {
    VK_DYNAMIC_STATE_LINE_WIDTH,
    VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_SCISSOR,
};

GraphicsPipeline::GraphicsPipeline(VkPipeline pipeline, VkPipelineLayout layout, VkDescriptorSetLayout uniformSetLayout, VkDescriptorSetLayout samplerSetLayout, ShaderType shaderType)
    : m_pipeline(pipeline),
      m_layout(layout),
      m_uniformsSetLayout(uniformSetLayout),
      m_samplerSetLayout(samplerSetLayout),
      m_shaderType(shaderType) {
    auto &context = vulkan::VulkanContext::get();
    auto &device= context.getDevice();

    VkDescriptorPool descriptorPool = context.getDescriptorPool();

    VkDescriptorSetAllocateInfo uniformSetAllocateInfo{};
    uniformSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    uniformSetAllocateInfo.descriptorPool = descriptorPool;
    uniformSetAllocateInfo.descriptorSetCount = 1;
    uniformSetAllocateInfo.pSetLayouts = &m_uniformsSetLayout;

    CHECK_VK_FUNCTION(vkAllocateDescriptorSets(device, &uniformSetAllocateInfo, &m_uniformSet));

    const auto stateBufferInfo = context.getUniformBuffer(vulkan::UniformBuffersHolder::STATE)->getBufferInfo();
    const auto fogBufferInfo = context.getUniformBuffer(vulkan::UniformBuffersHolder::FOG)->getBufferInfo();
    const auto projectionViewBufferInfo = context.getUniformBuffer(vulkan::UniformBuffersHolder::PROJECTION_VIEW)->getBufferInfo();
    const auto backgraundUniformInfo = context.getUniformBuffer(vulkan::UniformBuffersHolder::BACKGROUND)->getBufferInfo();
    const auto skyboxUniformInfo = context.getUniformBuffer(vulkan::UniformBuffersHolder::SKYBOX)->getBufferInfo();
    const auto applyUniformBufferInfo = context.getUniformBuffer(vulkan::UniformBuffersHolder::APPLY)->getBufferInfo();

    switch (shaderType) {
        case ShaderType::MAIN: {
            const std::array writeSets = {
                initializers::writeUniformBufferDescriptorSet(m_uniformSet, 0, &stateBufferInfo),
                initializers::writeUniformBufferDescriptorSet(m_uniformSet, 1, &fogBufferInfo),
            };

            vkUpdateDescriptorSets(device, writeSets.size(), writeSets.data(), 0, nullptr);
        } break;
        case ShaderType::LINES: {
            const std::array writeSets = {
                initializers::writeUniformBufferDescriptorSet(m_uniformSet, 0, &projectionViewBufferInfo),
            };

            vkUpdateDescriptorSets(device, writeSets.size(), writeSets.data(), 0, nullptr);
        } break;
        case ShaderType::UI: {
            const std::array writeSets = {
                initializers::writeUniformBufferDescriptorSet(m_uniformSet, 0, &projectionViewBufferInfo),
            };

            vkUpdateDescriptorSets(device, writeSets.size(), writeSets.data(), 0, nullptr);
        } break;
        case ShaderType::BACKGROUND: {
            const std::array writeSets = {
                initializers::writeUniformBufferDescriptorSet(m_uniformSet, 0, &backgraundUniformInfo),
            };

            vkUpdateDescriptorSets(device, writeSets.size(), writeSets.data(), 0, nullptr);
        } break;
        case ShaderType::SKYBOX_GEN: {
            const std::array writeSets = {
                initializers::writeUniformBufferDescriptorSet(m_uniformSet, 0, &skyboxUniformInfo),
            };

            vkUpdateDescriptorSets(device, writeSets.size(), writeSets.data(), 0, nullptr);
        } break;
        case ShaderType::UI3D: {
            const std::array writeSets = {
                initializers::writeUniformBufferDescriptorSet(m_uniformSet, 0, &projectionViewBufferInfo),
                initializers::writeUniformBufferDescriptorSet(m_uniformSet, 1, &applyUniformBufferInfo)
            };

            vkUpdateDescriptorSets(device, writeSets.size(), writeSets.data(), 0, nullptr);
        } break;
        case ShaderType::NONE:
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

VkPipelineLayout GraphicsPipeline::getLayout() const {
    return m_layout;
}

VkDescriptorSet GraphicsPipeline::getUniformSet() const {
    return m_uniformSet;
}

VkDescriptorSet GraphicsPipeline::getSamplerSet() const {
    return m_samplerSet;
}

void GraphicsPipeline::bind(VkCommandBuffer commandBuffer, VkExtent2D extent2D) {
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
    const VkDescriptorSet sets[] = { m_uniformSet };
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_layout, 0, 1, sets, 0, nullptr);

    VkViewport viewport{};
    viewport.width = static_cast<float>(extent2D.width);
    viewport.height = -static_cast<float>(extent2D.height);
    viewport.x = 0.0f;
    viewport.y = static_cast<float>(extent2D.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.extent = extent2D;
    scissor.offset = { 0, 0 };
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
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
        case ShaderType::UI:
        case ShaderType::UI3D:
        case ShaderType::BACKGROUND:
            // descriptorBindingFlags.emplace_back(VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT);
        break;
        case ShaderType::LINES:
        default:
            break;
    }

    auto &device = vulkan::VulkanContext::get().getDevice();

    // VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsCreateInfo{};
    // bindingFlagsCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
    // bindingFlagsCreateInfo.bindingCount = descriptorBindingFlags.size();
    // bindingFlagsCreateInfo.pBindingFlags = descriptorBindingFlags.data();

    VkDescriptorSetLayoutCreateInfo uniformsSetLayoutCreateInfo{};
    uniformsSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    uniformsSetLayoutCreateInfo.bindingCount = uniformBindings.size();
    uniformsSetLayoutCreateInfo.pBindings = uniformBindings.data();

    VkDescriptorSetLayoutCreateInfo samplerSetLayoutCreateInfo{};
    samplerSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    samplerSetLayoutCreateInfo.bindingCount = samplerBindings.size();
    samplerSetLayoutCreateInfo.pBindings = samplerBindings.data();
    samplerSetLayoutCreateInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;

    VkDescriptorSetLayout uniformSetLayout = VK_NULL_HANDLE;
    CHECK_VK_FUNCTION(vkCreateDescriptorSetLayout(device, &uniformsSetLayoutCreateInfo, nullptr, &uniformSetLayout));

    VkDescriptorSetLayout samplerSetLayout = VK_NULL_HANDLE;
    CHECK_VK_FUNCTION(vkCreateDescriptorSetLayout(device, &samplerSetLayoutCreateInfo, nullptr, &samplerSetLayout));

    std::array setLayouts = { uniformSetLayout, samplerSetLayout };

    std::array pushConstantRanges = getPushConstantRanges(tools::PushConstantRange{ sizeof(DynamicConstants), 0, VK_SHADER_STAGE_VERTEX_BIT });

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = setLayouts.size();
    pipelineLayoutCreateInfo.pSetLayouts = setLayouts.data();
    pipelineLayoutCreateInfo.pushConstantRangeCount = type == ShaderType::MAIN ? pushConstantRanges.size() : 0;
    pipelineLayoutCreateInfo.pPushConstantRanges = pushConstantRanges.data();

    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    CHECK_VK_FUNCTION(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout));

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
    inputAssembly.topology = type == ShaderType::LINES ? VK_PRIMITIVE_TOPOLOGY_LINE_LIST : VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    const VkExtent2D extent2D = vulkan::VulkanContext::get().getSwapchain().getExtent();

    VkViewport viewport{};
    viewport.x = 0.f;
    viewport.y = 0.f;
    viewport.width = static_cast<float>(extent2D.width);
    viewport.height = static_cast<float>(extent2D.height);
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
    rasterizer.polygonMode = type == ShaderType::LINES ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = type == ShaderType::BACKGROUND ? VK_FRONT_FACE_CLOCKWISE : VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = type == ShaderType::UI || type == ShaderType::SKYBOX_GEN ? VK_FALSE : VK_TRUE;
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
    const auto depthStencilFormat = vulkan::VulkanContext::get().getDepth().getFormat();

    VkFormat cubeFormat = VK_FORMAT_R8G8B8A8_UNORM;
    VkPipelineRenderingCreateInfo renderingCreateInfo{};
    renderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    renderingCreateInfo.colorAttachmentCount = 1;
    renderingCreateInfo.pColorAttachmentFormats = type == ShaderType::SKYBOX_GEN ? &cubeFormat : &sawpchainFormat;
    renderingCreateInfo.depthAttachmentFormat = depthStencilFormat;
    renderingCreateInfo.stencilAttachmentFormat = depthStencilFormat;
    renderingCreateInfo.viewMask = type == ShaderType::SKYBOX_GEN ? 0b111111 : 0;

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
    CHECK_VK_FUNCTION(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, nullptr, &pipeline));

    return std::make_shared<GraphicsPipeline>(pipeline, pipelineLayout, uniformSetLayout, samplerSetLayout, type);
}
