//
// Created by chelovek on 11/28/23.
//

#include "Skybox.h"

#include <array>
#include "../graphics-common/IShader.h"
#include "device/Shader.h"
#include "uniforms/SkyboxUniform.h"

#ifndef M_PI
#define M_PI 3.141592
#endif // M_PI

namespace vulkan {
    Skybox::Skybox(uint size, IShader* shader)
        : m_shader(shader),
          m_cubemap(size, size, VK_FORMAT_R8G8B8A8_SRGB) {

        VertexBackSkyGen vertices[] = {
            {{-1.f, -1.f}}, {{-1.0f, 1.0f}}, {{1.0f, 1.0f}},
            {{-1.f, -1.f}}, {{1.0f, 1.0f}}, {{1.0f, -1.0f}}
        };

        m_mesh = new Mesh(vertices, 6);
    }

    Skybox::~Skybox() {
        m_cubemap.destroy();
        delete m_mesh;
    }

    void Skybox::draw(IShader* shader) {
        bind();
        m_mesh->bind();
        m_mesh->draw({0, 6});
    }

    void Skybox::draw(IShader* shader, VkCommandBuffer commandBuffer) {
        bind(commandBuffer, dynamic_cast<Shader *>(shader)->getOrCreatePipeline());
        m_mesh->bind(commandBuffer);
        m_mesh->draw({0, 6}, commandBuffer);
    }

    void Skybox::refresh(float t, float mie, uint quality) {
        m_ready = true;
        constexpr std::array<glm::vec3, 6> xaxs = {
            glm::vec3{0.0f, 0.0f, -1.0f},
            glm::vec3{0.0f, 0.0f, 1.0f},
            glm::vec3{-1.0f, 0.0f, 0.0f},

            glm::vec3{-1.0f, 0.0f, 0.0f},
            glm::vec3{-1.0f, 0.0f, 0.0f},
            glm::vec3{1.0f, 0.0f, 0.0f},
        };
        constexpr std::array<glm::vec3, 6> yaxs = {
            glm::vec3{0.0f, 1.0f, 0.0f},
            glm::vec3{0.0f, 1.0f, 0.0f},
            glm::vec3{0.0f, 0.0f, -1.0f},

            glm::vec3{0.0f, 0.0f, 1.0f},
            glm::vec3{0.0f, 1.0f, 0.0f},
            glm::vec3{0.0f, 1.0f, 0.0f},
        };

        constexpr std::array<glm::vec3, 6> zaxs = {
            glm::vec3{1.0f, 0.0f, 0.0f},
            glm::vec3{-1.0f, 0.0f, 0.0f},
            glm::vec3{0.0f, -1.0f, 0.0f},

            glm::vec3{0.0f, 1.0f, 0.0f},
            glm::vec3{0.0f, 0.0f, -1.0f},
            glm::vec3{0.0f, 0.0f, 1.0f},
        };
        t *= M_PI * 2.0f;

        SkyboxUniform skyboxUniform{};

        for (size_t i = 0; i < 6; ++i) {
            skyboxUniform.xaxis[i].val = xaxs[i];
            skyboxUniform.yaxis[i].val = yaxs[i];
            skyboxUniform.zaxis[i].val = zaxs[i];
        }

        skyboxUniform.quality = static_cast<int>(quality);
        skyboxUniform.mie = mie;
        skyboxUniform.fog = mie - 1.0f;
        skyboxUniform.lightDir = glm::normalize(glm::vec3(sin(t), -cos(t), 0.7f));

        auto &context = VulkanContext::get();
        auto *skyboxBuffer = context.getUniformBuffer(UniformBuffersHolder::SKYBOX);
        skyboxBuffer->uploadData(skyboxUniform);

        const auto commandBuffer = context.beginDrawSkybox(m_cubemap, 0, 0, 0);
        m_shader->use(commandBuffer, {Image::getWidth(m_cubemap), Image::getHeight(m_cubemap)});

        VkViewport viewport{};
        viewport.width = static_cast<float>(Image::getWidth(m_cubemap));
        viewport.height = static_cast<float>(Image::getHeight(m_cubemap));
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        m_mesh->bind(commandBuffer);
        m_mesh->draw({0, 6}, commandBuffer);

        context.endDrawSkybox(m_cubemap, commandBuffer);
    }

    void Skybox::bind(VkCommandBuffer commandBuffer, GraphicsPipeline *pipeline) {
        m_cubemap.bind(commandBuffer, pipeline);
    }

    void Skybox::bind() {
        m_cubemap.bind();
    }

    bool Skybox::isReady() const {
        return m_ready;
    }
} // vulkan