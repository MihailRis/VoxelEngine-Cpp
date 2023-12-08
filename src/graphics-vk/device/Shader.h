//
// Created by chelovek on 11/21/23.
//

#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <vector>
#include <filesystem>
#include <glm/fwd.hpp>
#include <vulkan/vulkan.h>

#include "../UniformValues.h"
#include "../ShaderType.h"
#include "../../graphics-common/IShader.h"

class GraphicsPipeline;

namespace vulkan {

    class Shader : public IShader {
        std::vector<VkShaderModule> m_modules;
        std::vector<VkPipelineShaderStageCreateInfo> m_stages;

        UniformValues m_values;
        ShaderType m_type;

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

        void pushConstant(const DynamicConstants& constants) override;

        void use(VkCommandBuffer commandBuffer, VkExtent2D extent2D) override;
    private:
        GraphicsPipeline *getOrCreatePipeline();
        void updateUniform();
    };

    Shader *loadShader(std::string vertexFile, std::string fragmentFile, ShaderType type);

} // vulkan

#endif //SHADER_H
