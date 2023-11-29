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
    CHECK_VK(vkCreateShaderModule(device, &shaderModuleCreateInfo, nullptr, &module));

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
        const VkCommandBuffer commandBuffer = VulkanContext::get().getCurrentState().commandbuffer;
        m_pipeline->bind(commandBuffer);
        VulkanContext::get().updateState(m_pipeline.get());
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

    GraphicsPipeline* Shader::getOrCreatePipeline() {
        if (m_pipeline == nullptr)
            m_pipeline = GraphicsPipeline::create(m_stages, m_type);

        return m_pipeline.get();
    }

    void Shader::updateUniform() {
        const auto &context = VulkanContext::get();
        auto *stateBuffer = context.getUniformBuffer(UniformBuffersHolder::STATE);
        auto *lightBuffer = context.getUniformBuffer(UniformBuffersHolder::LIGHT);
        auto *fogBuffer = context.getUniformBuffer(UniformBuffersHolder::FOG);
        auto *projectionViewBuffer = context.getUniformBuffer(UniformBuffersHolder::PROJECTION_VIEW);
        auto *backgroundBuffer = context.getUniformBuffer(UniformBuffersHolder::BACKGROUND);
        auto *skyboxBuffer = context.getUniformBuffer(UniformBuffersHolder::SKYBOX);

        const auto stateUniform = m_values.getStateUniform();
        const auto lightUniform = m_values.getLightUniform();
        const auto fogUniform = m_values.getFogUniform();
        const auto projectionViewUniform = m_values.getProjectionView();
        const auto backgroundUniform = m_values.getBackgroundUniform();
        const auto skyboxUniform = m_values.getSkyboxUniform();

        stateBuffer->uploadData(stateUniform);
        lightBuffer->uploadData(lightUniform);
        fogBuffer->uploadData(fogUniform);
        projectionViewBuffer->uploadData(projectionViewUniform);
        backgroundBuffer->uploadData(backgroundUniform);
        skyboxBuffer->uploadData(skyboxUniform);
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