//
// Created by chelovek on 11/28/23.
//

#include "Skybox.h"
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
        shader->uniform1i("u_cubemap", 1);
        bind();
        m_mesh->draw({0, 6});
    }

    void Skybox::refresh(float t, float mie, uint quality) {
        m_ready = true;
        m_shader->use();
        const glm::vec3 xaxs[] = {
            {0.0f, 0.0f, -1.0f},
            {0.0f, 0.0f, 1.0f},
            {-1.0f, 0.0f, 0.0f},

            {-1.0f, 0.0f, 0.0f},
            {-1.0f, 0.0f, 0.0f},
            {1.0f, 0.0f, 0.0f},
        };
        const glm::vec3 yaxs[] = {
            {0.0f, 1.0f, 0.0f},
            {0.0f, 1.0f, 0.0f},
            {0.0f, 0.0f, -1.0f},

            {0.0f, 0.0f, 1.0f},
            {0.0f, 1.0f, 0.0f},
            {0.0f, 1.0f, 0.0f},
        };

        const glm::vec3 zaxs[] = {
            {1.0f, 0.0f, 0.0f},
            {-1.0f, 0.0f, 0.0f},
            {0.0f, -1.0f, 0.0f},

            {0.0f, 1.0f, 0.0f},
            {0.0f, 0.0f, -1.0f},
            {0.0f, 0.0f, 1.0f},
        };
        t *= M_PI*2.0f;

        m_shader->uniform1i("u_quality", quality);
        m_shader->uniform1f("u_mie", mie);
        m_shader->uniform3f("u_lightDir", glm::normalize(glm::vec3(sin(t), -cos(t), 0.7f)));
    }

    void Skybox::bind() const {
    }

    void Skybox::unbind() const {
        // nothing
    }

    bool Skybox::isReady() const {
        return m_ready;
    }
} // vulkan