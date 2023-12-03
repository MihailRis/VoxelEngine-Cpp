//
// Created by chelovek on 11/28/23.
//

#include "Skybox.h"

#include <array>
#include "../graphics-base/IShader.h"

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
        m_mesh->draw({0, 6});
    }

    void Skybox::refresh(float t, float mie, uint quality) {
        m_ready = true;
        m_shader->use();
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

        m_shader->uniform1i("u_quality", quality);
        m_shader->uniform1f("u_mie", mie);
        m_shader->uniform3f("u_lightDir", glm::normalize(glm::vec3(sin(t), -cos(t), 0.7f)));
    }

    void Skybox::bind() {
        m_cubemap.bind();
    }

    void Skybox::unbind() const {
        // nothing
    }

    bool Skybox::isReady() const {
        return m_ready;
    }
} // vulkan