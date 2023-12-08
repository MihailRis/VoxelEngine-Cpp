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

inline std::vector<char> readFile(const std::filesystem::path &path) {

    std::ifstream file(path, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        return {};
    }

    const ssize_t fsize = file.tellg();

    std::vector<char> buffer(fsize);

    file.seekg(0);
    file.read(buffer.data(), fsize);

    file.close();

    return buffer;
}

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

    Shader::Shader(VkShaderModule vertexModule, VkShaderModule fragmentModule, ShaderType type) : m_type(type) {
        m_modules.emplace_back(vertexModule);
        m_modules.emplace_back(fragmentModule);

        m_stages.emplace_back(initializers::pipelineShaderStageCreateInfo(vertexModule, VK_SHADER_STAGE_VERTEX_BIT));
        m_stages.emplace_back(initializers::pipelineShaderStageCreateInfo(fragmentModule, VK_SHADER_STAGE_FRAGMENT_BIT));

        m_pipeline = GraphicsPipeline::create(m_stages, m_type);
    }

    Shader::~Shader() {
        auto &device = VulkanContext::get().getDevice();
        for (const auto &module : m_modules) {
            vkDestroyShaderModule(device, module, nullptr);
        }
    }

    void Shader::use() {
        if (m_pipeline == nullptr) getOrCreatePipeline();
        auto &context = VulkanContext::get();
        auto &state = context.getCurrentState();
        if (state.commandbuffer == VK_NULL_HANDLE) return;
        m_pipeline->bind(state.commandbuffer, state.viewport);
        context.updateState(m_pipeline.get());
    }

    void Shader::uniformMatrix(std::string name, glm::mat4 matrix) {
        m_values.addOrUpdate(name, matrix);
        updateUniform();
    }

    void Shader::uniform1i(std::string name, int x) {
        m_values.addOrUpdate(name, x);
        updateUniform();
    }

    void Shader::uniform1f(std::string name, float x) {
        m_values.addOrUpdate(name, x);
        updateUniform();
    }

    void Shader::uniform2f(std::string name, float x, float y) {
        m_values.addOrUpdate(name, glm::vec2(x, y));
        updateUniform();
    }

    void Shader::uniform2f(std::string name, glm::vec2 xy) {
        m_values.addOrUpdate(name, xy);
        updateUniform();
    }

    void Shader::uniform3f(std::string name, float x, float y, float z) {
        m_values.addOrUpdate(name, glm::vec3(x, y, z));
        updateUniform();
    }

    void Shader::uniform3f(std::string name, glm::vec3 xyz) {
        m_values.addOrUpdate(name, xyz);
        updateUniform();
    }

    void Shader::pushConstant(const DynamicConstants &constants) {
        const auto commandBuffer = VulkanContext::get().getCurrentState().commandbuffer;

        if (commandBuffer == VK_NULL_HANDLE || m_pipeline == nullptr || m_type != ShaderType::MAIN) return;
        vkCmdPushConstants(commandBuffer, m_pipeline->getLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(DynamicConstants), &constants);
    }

    void Shader::use(VkCommandBuffer commandBuffer, VkExtent2D extent2D) {
        if (commandBuffer == VK_NULL_HANDLE) return;
        m_pipeline->bind(commandBuffer, extent2D);
    }

    GraphicsPipeline* Shader::getOrCreatePipeline() {
        if (m_pipeline == nullptr)
            m_pipeline = GraphicsPipeline::create(m_stages, m_type);

        return m_pipeline.get();
    }

    // TODO: optimize this
    void Shader::updateUniform() {
        auto &context = VulkanContext::get();
        auto *stateBuffer = context.getUniformBuffer(UniformBuffersHolder::STATE);
        auto *fogBuffer = context.getUniformBuffer(UniformBuffersHolder::FOG);
        auto *projectionViewBuffer = context.getUniformBuffer(UniformBuffersHolder::PROJECTION_VIEW);
        auto *backgroundBuffer = context.getUniformBuffer(UniformBuffersHolder::BACKGROUND);
        auto *applyBuffer = context.getUniformBuffer(UniformBuffersHolder::APPLY);

        const auto stateUniform = m_values.getStateUniform();
        const auto fogUniform = m_values.getFogUniform();
        const auto projectionViewUniform = m_values.getProjectionView();
        const auto backgroundUniform = m_values.getBackgroundUniform();
        const auto applyUniform = m_values.getApplyUniform();

        switch (m_type) {
            case ShaderType::NONE:
                return;
            case ShaderType::MAIN: {
                stateBuffer->uploadData(stateUniform);
                fogBuffer->uploadData(fogUniform);
            } break;
            case ShaderType::LINES:
            case ShaderType::UI: {
                projectionViewBuffer->uploadData(projectionViewUniform);
            } break;
            case ShaderType::BACKGROUND: {
                backgroundBuffer->uploadData(backgroundUniform);
            } break;
            case ShaderType::UI3D: {
                projectionViewBuffer->uploadData(projectionViewUniform);
                applyBuffer->uploadData(applyUniform);
            } break;
        }
    }

    Shader* loadShader(std::string vertexFile, std::string fragmentFile, ShaderType type) {
        if (type == ShaderType::NONE)
            return nullptr;

        auto &device = VulkanContext::get().getDevice();
        const std::vector<char> vertexCode = readFile(vertexFile);
        const std::vector<char> fragmentCode = readFile(fragmentFile);

        if (vertexCode.empty() || fragmentCode.empty())
            return nullptr;

        auto vertexModule = createModule(vertexCode, device);
        auto fragmentModule = createModule(fragmentCode, device);

        return new Shader(vertexModule, fragmentModule, type);
    }
} // vulkan