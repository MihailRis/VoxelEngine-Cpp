//
// Created by chelovek on 11/21/23.
//

#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <unordered_map>
#include <filesystem>
#include <glm/fwd.hpp>
#include <vulkan/vulkan.h>

#include "UniformBuffer.h"
#include "../UniformValues.h"
#include "../ShaderType.h"
#include "../../graphics-common/IShader.h"
#include "../uniforms/SkyboxUniform.h"

class GraphicsPipeline;

namespace vulkan {

    class Shader : public IShader {
        std::vector<VkShaderModule> m_modules;
        std::vector<VkPipelineShaderStageCreateInfo> m_stages;
        std::unordered_map<std::string, std::unique_ptr<UniformBuffer>> m_uniformBuffers;

        UniformValues m_values;
        ShaderType m_type;
        
        uint32_t m_dynamicCount = 0;
        bool m_hasDynamic = false;

        std::shared_ptr<GraphicsPipeline> m_pipeline = nullptr;
    public:
        Shader(VkShaderModule vertexModule, VkShaderModule fragmentModule, ShaderType type);
        ~Shader() override;

        void use() override;

        void uniformMatrix(std::string name, glm::mat4 matrix) override;

        void uniform1i(std::string name, int x) override;

        void uniform1f(std::string name, float x) override;

        void uniform2f(std::string name, float x, float y) override;

        void uniform2f(std::string name, glm::vec2 xy) override;

        void uniform3f(std::string name, float x, float y, float z) override;

        void uniform3f(std::string name, glm::vec3 xyz) override;

        void uniform(const StateUniform &uniform) override;
        void uniform(const FogUniform &uniform) override;
        void uniform(const ApplyUniform &uniform) override;
        void uniform(const ProjectionViewUniform &uniform) override;
        void uniform(const SkyboxUniform &uniform) override;
        void uniform(const BackgroundUniform &uniform) override;

        void pushConstant(const DynamicConstants& constants) override;

        void use(VkCommandBuffer commandBuffer, VkExtent2D extent2D) override;
        
        GraphicsPipeline *getPipeline() const;
    };

    Shader *loadShader(const std::vector<char> &vertexFile, const std::vector<char> &fragmentFile, ShaderType type);

} // vulkan

#endif //SHADER_H
