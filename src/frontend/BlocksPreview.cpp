#include "BlocksPreview.h"

#include <glm/ext.hpp>

#include "../graphics/Viewport.h"
#include "../graphics/Shader.h"
#include "../graphics/Texture.h"
#include "../graphics/Atlas.h"

#ifdef USE_VULKAN
#include "../graphics-vk/uniforms/ApplyUniform.h"
#include "../graphics-vk/uniforms/ProjectionViewUniform.h"
#endif

#include "../window/Camera.h"
#include "../voxels/Block.h"
#include "ContentGfxCache.h"

BlocksPreview::BlocksPreview(IShader* shader,
                             Atlas* atlas, 
                             const ContentGfxCache* cache)
    : shader(shader), atlas(atlas), cache(cache) {
    batch = std::make_unique<Batch3D>(1024);
}

BlocksPreview::~BlocksPreview() {
}

void BlocksPreview::begin(const Viewport* viewport) {
    this->viewport = viewport;
    shader->use();
#ifdef USE_VULKAN
    const ProjectionViewUniform projectionViewUniform = {
        glm::ortho(0.0f, static_cast<float>(viewport->getWidth()),
                   0.0f, static_cast<float>(viewport->getHeight()),
                    -1000.0f, 1000.0f) *
        glm::lookAt(glm::vec3(2, 2, 2), glm::vec3(0.0f), glm::vec3(0, 1, 0))
    };

    shader->uniform(projectionViewUniform);
#else
    shader->uniformMatrix("u_projview",
        glm::ortho(0.0f, float(viewport->getWidth()),
                   0.0f, float(viewport->getHeight()),
                    -1000.0f, 1000.0f) *
        glm::lookAt(glm::vec3(2, 2, 2), glm::vec3(0.0f), glm::vec3(0, 1, 0)));
#endif
    batch->begin();
    batch->texture(atlas->getTexture());
}

/* Draw one block preview at given screen position */
void BlocksPreview::draw(const Block* def, int x, int y, int size, glm::vec4 tint) {
    uint width = viewport->getWidth();
    uint height = viewport->getHeight();

    y = height - y - 1;
    x += 2;
    y -= 35;

#ifdef USE_VULKAN
    glm::vec3 offset (static_cast<float>(x) / static_cast<float>(width) * 2, static_cast<float>(y) / static_cast<float>(height) * 2, 0.0f);
    const ApplyUniform applyUniform = {glm::translate(glm::mat4(1.0f), offset)};
    shader->uniform(applyUniform);
#else
    glm::vec3 offset (x/float(width) * 2, y/float(height) * 2, 0.0f);
    shader->uniformMatrix("u_apply", glm::translate(glm::mat4(1.0f), offset));
#endif

    blockid_t id = def->rt.id;
    const UVRegion texfaces[6]{ cache->getRegion(id, 0), cache->getRegion(id, 1),
                                cache->getRegion(id, 2), cache->getRegion(id, 3),
                                cache->getRegion(id, 4), cache->getRegion(id, 5)};

    switch (def->model) {
        case BlockModel::none:
            // something went wrong...
            break;
        case BlockModel::block:
            batch->blockCube(glm::vec3(static_cast<float>(size) * 0.63f), texfaces, tint, !def->rt.emissive);
            break;
        case BlockModel::aabb:
            batch->blockCube(def->hitbox.size() * glm::vec3(static_cast<float>(size) * 0.63f),
                             texfaces, tint, !def->rt.emissive);
            break;
        case BlockModel::xsprite: {
            glm::vec3 right = glm::normalize(glm::vec3(1.f, 0.f, -1.f));
            batch->sprite(right * static_cast<float>(size) * 0.43f + glm::vec3(0, static_cast<float>(size) * 0.4f, 0),
                          glm::vec3(0.f, 1.f, 0.f),
                          right,
                          static_cast<float>(size) * 0.5f, static_cast<float>(size) * 0.6f,
                          texfaces[0],
                          tint);
            break;
        }
    }

    batch->flush();
}

void BlocksPreview::end() {
    batch->end();
}
