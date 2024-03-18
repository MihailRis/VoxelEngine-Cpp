#include "Skybox.h"

#include <iostream>
#include <GL/glew.h>
#include <glm/glm.hpp>

#include "../../assets/Assets.h"
#include "../../graphics/core/Shader.h"
#include "../../graphics/core/Mesh.h"
#include "../../graphics/core/Batch3D.h"
#include "../../graphics/core/Texture.h"
#include "../../graphics/core/Cubemap.h"
#include "../../graphics/core/Framebuffer.h"
#include "../../graphics/core/GfxContext.h"
#include "../../window/Window.h"
#include "../../window/Camera.h"

#ifndef M_PI
#define M_PI 3.141592
#endif // M_PI

const int STARS_COUNT = 3000;
const int STARS_SEED = 632;

Skybox::Skybox(uint size, Shader* shader) 
  : size(size), 
    shader(shader), 
    batch3d(std::make_unique<Batch3D>(4096)) 
{
    auto cubemap = std::make_unique<Cubemap>(size, size, ImageFormat::rgb888);

    uint fboid;
    glGenFramebuffers(1, &fboid);
    fbo = std::make_unique<Framebuffer>(fboid, 0, std::move(cubemap));

    float vertices[] {
        -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f,  1.0f, 1.0f, 1.0f, -1.0f
    };
    vattr attrs[] {{2}, {0}};
    mesh = std::make_unique<Mesh>(vertices, 6, attrs);

    sprites.push_back(skysprite {
        "misc/moon",
        M_PI*0.5f,
        4.0f,
        false
    });

    sprites.push_back(skysprite {
        "misc/sun",
        M_PI*1.5f,
        4.0f,
        true
    });
}

Skybox::~Skybox() {
}

void Skybox::drawBackground(Camera* camera, Assets* assets, int width, int height) {
    Shader* backShader = assets->getShader("background");
    backShader->use();
    backShader->uniformMatrix("u_view", camera->getView(false));
    backShader->uniform1f("u_zoom", camera->zoom*camera->getFov()/(M_PI*0.5f));
    backShader->uniform1f("u_ar", float(width)/float(height));
    backShader->uniform1i("u_cubemap", 1);
    bind();
    mesh->draw();
    unbind();
}

void Skybox::drawStars(float angle, float opacity) {
    batch3d->texture(nullptr);
    random.setSeed(STARS_SEED);
    for (int i = 0; i < STARS_COUNT; i++) {
        float rx = (random.randFloat()) - 0.5f;
        float ry = (random.randFloat()) - 0.5f;
        float z = (random.randFloat()) - 0.5f;
        float x = rx * sin(angle) + ry * -cos(angle);
        float y = rx * cos(angle) + ry * sin(angle);

        float sopacity = random.randFloat();
        if (y < 0.0f)
            continue;

        sopacity *= (0.2f+sqrt(cos(angle))*0.5) - 0.05;
        glm::vec4 tint (1,1,1, sopacity * opacity);
        batch3d->point(glm::vec3(x, y, z), tint);
    }
    batch3d->flushPoints();
}

void Skybox::draw(
    const GfxContext& pctx, 
    Camera* camera, 
    Assets* assets, 
    float daytime,
    float fog) 
{
    const Viewport& viewport = pctx.getViewport();
    int width = viewport.getWidth();
    int height = viewport.getHeight();

    drawBackground(camera, assets, width, height);

    GfxContext ctx = pctx.sub();
    ctx.setBlendMode(blendmode::addition);

    Shader* shader = assets->getShader("ui3d");
    shader->use();
    shader->uniformMatrix("u_projview", camera->getProjView(false));
    shader->uniformMatrix("u_apply", glm::mat4(1.0f));
    batch3d->begin();

    float angle = daytime * M_PI * 2;
    float opacity = glm::pow(1.0f-fog, 7.0f);
    
    for (auto& sprite : sprites) {
        batch3d->texture(assets->getTexture(sprite.texture));

        float sangle = daytime * M_PI*2 + sprite.phase;
        float distance = sprite.distance;

        glm::vec3 pos(-cos(sangle)*distance, sin(sangle)*distance, 0);
        glm::vec3 up(-sin(-sangle), cos(-sangle), 0.0f);
        glm::vec4 tint (1,1,1, opacity);
        if (!sprite.emissive) {
            tint *= 0.6f+cos(angle)*0.4;
        }
        batch3d->sprite(pos, glm::vec3(0, 0, 1), 
                        up, 1, 1, UVRegion(), tint);
    }

    drawStars(angle, opacity);
}

void Skybox::refresh(const GfxContext& pctx, float t, float mie, uint quality) {
    GfxContext ctx = pctx.sub();
    ctx.setDepthMask(false);
    ctx.setDepthTest(false);
    ctx.setFramebuffer(fbo.get());
    ctx.setViewport(Viewport(size, size));

    auto cubemap = dynamic_cast<Cubemap*>(fbo->getTexture());

    ready = true;
    glActiveTexture(GL_TEXTURE1);
    cubemap->bind();
    shader->use();

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
    
    shader->uniform1i("u_quality", quality);
    shader->uniform1f("u_mie", mie);
    shader->uniform1f("u_fog", mie - 1.0f);
    shader->uniform3f("u_lightDir", glm::normalize(glm::vec3(sin(t), -cos(t), 0.0f)));
    for (uint face = 0; face < 6; face++) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, cubemap->getId(), 0);
        shader->uniform3f("u_xaxis", xaxs[face]);
        shader->uniform3f("u_yaxis", yaxs[face]);
        shader->uniform3f("u_zaxis", zaxs[face]);
        mesh->draw();
    }
    cubemap->unbind();
    glActiveTexture(GL_TEXTURE0);
}

void Skybox::bind() const {
    glActiveTexture(GL_TEXTURE1);
    fbo->getTexture()->bind();
    glActiveTexture(GL_TEXTURE0);
}

void Skybox::unbind() const {
    glActiveTexture(GL_TEXTURE1);
    fbo->getTexture()->unbind();
    glActiveTexture(GL_TEXTURE0);
}
