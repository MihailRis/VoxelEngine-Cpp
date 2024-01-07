//
// Created by chelovek on 11/21/23.
//

#include "Shader.h"

#include <iostream>
#include <fstream>

#include "GraphicsPipeline.h"
#include "../Inistializers.h"
#include "../VulkanContext.h"
#include "../VulkanDefenitions.h"
#include "../uniforms/DynamicConstants.h"
#include "../uniforms/SkyboxUniform.h"

inline VkShaderModule createModule(const std::vector<char> &code, VkDevice device) {
    VkShaderModuleCreateInfo shaderModuleCreateInfo{};
    shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleCreateInfo.codeSize = code.size();
    shaderModuleCreateInfo.pCode = reinterpret_cast<const u32 *>(code.data());

    VkShaderModule module = VK_NULL_HANDLE;
    CHECK_VK_FUNCTION(vkCreateShaderModule(device, &shaderModuleCreateInfo, nullptr, &module));

    return module;
}

namespace vulkan {
    constexpr u32 OBJECT_COUNT = 32;

    Shader::Shader(VkShaderModule vertexModule, VkShaderModule fragmentModule, ShaderType type) : m_type(type) {
        m_modules.emplace_back(vertexModule);
        m_modules.emplace_back(fragmentModule);

        m_stages.emplace_back(initializers::pipelineShaderStageCreateInfo(vertexModule, VK_SHADER_STAGE_VERTEX_BIT));
        m_stages.emplace_back(initializers::pipelineShaderStageCreateInfo(fragmentModule, VK_SHADER_STAGE_FRAGMENT_BIT));

        const Device &device = VulkanContext::get().getDevice();
        const size_t projectionViewAlignment = device.padUniformBufferSize(sizeof(ProjectionViewUniform));

        switch (type) {
            case ShaderType::NONE:
                throw std::runtime_error("Failed to init shader");
            case ShaderType::MAIN: {
                m_uniformBuffers["fog"] = std::make_unique<UniformBuffer>(sizeof(FogUniform));
                m_uniformBuffers["state"] = std::make_unique<UniformBuffer>(sizeof(StateUniform));
            } break;
            case ShaderType::LINES:
            case ShaderType::UI: {
                m_uniformBuffers["projview"] = std::make_unique<UniformBuffer>(projectionViewAlignment, OBJECT_COUNT);
                m_hasDynamic = true;
            } break;
            case ShaderType::BACKGROUND: {
                m_uniformBuffers["background"] = std::make_unique<UniformBuffer>(sizeof(BackgroundUniform));
            } break;
            case ShaderType::SKYBOX_GEN: {
                m_uniformBuffers["skybox"] = std::make_unique<UniformBuffer>(sizeof(SkyboxUniform));
            } break;
            case ShaderType::UI3D: {
                m_uniformBuffers["apply"] = std::make_unique<UniformBuffer>(sizeof(ApplyUniform));
                m_uniformBuffers["projview"] = std::make_unique<UniformBuffer>(projectionViewAlignment, OBJECT_COUNT);
                m_hasDynamic = true;
            } break;
        }

        std::vector<initializers::UniformBufferInfo> bufferInfos;
        bufferInfos.reserve(m_uniformBuffers.size());

        for (const auto &buffer: m_uniformBuffers) {
            bufferInfos.emplace_back(buffer.second->getBufferInfo());
        }

        m_pipeline = GraphicsPipeline::create(m_stages, bufferInfos, m_type);
    }

    Shader::~Shader() {
        const Device &device = VulkanContext::get().getDevice();
        for (const auto &module : m_modules) {
            vkDestroyShaderModule(device, module, nullptr);
        }
    }

    void Shader::use() {
        auto &context = VulkanContext::get();
        auto &state = context.getCurrentState();
        if (state.commandBuffer == VK_NULL_HANDLE) return;
        m_pipeline->bind(state.commandBuffer, state.viewport);
        context.updateState(m_pipeline.get());
        if (!m_hasDynamic) {
            m_pipeline->bindDiscriptorSet(state.commandBuffer);
        }
    }

    void Shader::uniformMatrix(std::string name, glm::mat4 matrix) {
        m_values.addOrUpdate(name, matrix);
    }

    void Shader::uniform1i(std::string name, int x) {
        m_values.addOrUpdate(name, x);
    }

    void Shader::uniform1f(std::string name, float x) {
        m_values.addOrUpdate(name, x);
    }

    void Shader::uniform2f(std::string name, float x, float y) {
        m_values.addOrUpdate(name, glm::vec2(x, y));
    }

    void Shader::uniform2f(std::string name, glm::vec2 xy) {
        m_values.addOrUpdate(name, xy);
    }

    void Shader::uniform3f(std::string name, float x, float y, float z) {
        m_values.addOrUpdate(name, glm::vec3(x, y, z));
    }

    void Shader::uniform3f(std::string name, glm::vec3 xyz) {
        m_values.addOrUpdate(name, xyz);
    }

    void Shader::uniform(const StateUniform& uniform) {
        m_uniformBuffers["state"]->uploadData(uniform);
    }

    void Shader::uniform(const FogUniform& uniform) {
        m_uniformBuffers["fog"]->uploadData(uniform);
    }

    void Shader::uniform(const ApplyUniform& uniform) {
        m_uniformBuffers["apply"]->uploadData(uniform);
    }

    void Shader::uniform(const ProjectionViewUniform& uniform) {
        const VulkanContext &ctx = VulkanContext::get();
        const uint32_t offset = ctx.getDevice().padUniformBufferSize(sizeof(ProjectionViewUniform)) * m_dynamicCount;

        m_uniformBuffers["projview"]->uploadDataDynamic(uniform, offset);

        const State &state = ctx.getCurrentState();
        if (state.commandBuffer == VK_NULL_HANDLE) return;
        m_pipeline->bindDiscriptorSet(state.commandBuffer, 1, &offset);

        m_dynamicCount = (m_dynamicCount + 1) % OBJECT_COUNT;
    }

    void Shader::uniform(const SkyboxUniform& uniform) {
        m_uniformBuffers["skybox"]->uploadData(uniform);
    }

    void Shader::uniform(const BackgroundUniform& uniform) {
        m_uniformBuffers["background"]->uploadData(uniform);
    }

    void Shader::pushConstant(const DynamicConstants &constants) {
        const auto commandBuffer = VulkanContext::get().getCurrentState().commandBuffer;

        if (commandBuffer == VK_NULL_HANDLE || m_pipeline == nullptr || m_type != ShaderType::MAIN) return;
        vkCmdPushConstants(commandBuffer, m_pipeline->getLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(DynamicConstants), &constants);
    }

    void Shader::use(VkCommandBuffer commandBuffer, VkExtent2D extent2D) {
        if (commandBuffer == VK_NULL_HANDLE) return;
        m_pipeline->bind(commandBuffer, extent2D);
        if (!m_hasDynamic) {
            m_pipeline->bindDiscriptorSet(commandBuffer);
        }
    }

    GraphicsPipeline* Shader::getPipeline() const {
        return m_pipeline.get();
    }

    Shader* loadShader(const std::vector<char> &vertexFile, const std::vector<char> &fragmentFile, ShaderType type) {
        if (type == ShaderType::NONE)
            return nullptr;

        auto &device = VulkanContext::get().getDevice();

        if (vertexFile.empty() || fragmentFile.empty())
            return nullptr;

        const auto vertexModule = createModule(vertexFile, device);
        const auto fragmentModule = createModule(fragmentFile, device);

        return new Shader(vertexModule, fragmentModule, type);
    }
} // vulkan