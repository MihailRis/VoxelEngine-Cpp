#include "Skybox.h"
#include <GL/glew.h>
#include <iostream>
#include <glm/glm.hpp>

#include "../../graphics/Shader.h"
#include "../../graphics/Mesh.h"
#include "../../window/Window.h"

#ifndef M_PI
#define M_PI 3.141592
#endif // M_PI

using glm::vec3;

Skybox::Skybox(uint size, Shader* shader) : size(size), shader(shader) {
    glGenTextures(1, &cubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    for (uint face = 0; face < 6; face++) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, GL_RGB, size, size, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    }
    glGenFramebuffers(1, &fbo);

    float vertices[] {
        -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f,  1.0f, 1.0f, 1.0f, -1.0f
    };
    vattr attrs[] {2, 0};
    mesh = new Mesh(vertices, 6, attrs);
}

Skybox::~Skybox() {
    glDeleteTextures(1, &cubemap);
    glDeleteFramebuffers(1, &fbo);
    delete mesh;
}

void Skybox::draw(Shader* shader) {
    shader->uniform1i("u_cubemap", 1);
    bind();
    mesh->draw();
    unbind();
}

void Skybox::refresh(float t, float mie, uint quality) {
    ready = true;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);
    shader->use();
    Window::viewport(0,0, size,size);
    const vec3 xaxs[] = {
        {0.0f, 0.0f, -1.0f},
        {0.0f, 0.0f, 1.0f},
        {-1.0f, 0.0f, 0.0f},

        {-1.0f, 0.0f, 0.0f},
        {-1.0f, 0.0f, 0.0f},
        {1.0f, 0.0f, 0.0f},
    };
    const vec3 yaxs[] = {
        {0.0f, 1.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, -1.0f},
        
        {0.0f, 0.0f, 1.0f},
        {0.0f, 1.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
    };

    const vec3 zaxs[] = {
        {1.0f, 0.0f, 0.0f},
        {-1.0f, 0.0f, 0.0f},
        {0.0f, -1.0f, 0.0f},
        
        {0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, -1.0f},
        {0.0f, 0.0f, 1.0f},
    };
    t *= M_PI*2.0f;
    
    shader->uniform1i("u_quality", quality);
    shader->uniform1f("u_mie", mie);
    shader->uniform1f("u_fog", mie - 1.0f);
    shader->uniform3f("u_lightDir", glm::normalize(vec3(sin(t), -cos(t), -0.7f)));
    for (uint face = 0; face < 6; face++) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, cubemap, 0);
        shader->uniform3f("u_xaxis", xaxs[face]);
        shader->uniform3f("u_yaxis", yaxs[face]);
        shader->uniform3f("u_zaxis", zaxs[face]);
        mesh->draw(GL_TRIANGLES);
    }
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    Window::viewport(0, 0, Window::width, Window::height);
}

void Skybox::bind() const {
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);
    glActiveTexture(GL_TEXTURE0);
}

void Skybox::unbind() const {
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    glActiveTexture(GL_TEXTURE0);
}