#include "BlocksPreview.h"

#include <glm/ext.hpp>

#include "../assets/Assets.h"
#include "../graphics/Viewport.h"
#include "../graphics/Shader.h"
#include "../graphics/Texture.h"
#include "../graphics/Atlas.h"
#include "../graphics/Batch3D.h"
#include "../window/Camera.h"
#include "../voxels/Block.h"
#include "ContentGfxCache.h"

BlocksPreview::BlocksPreview(Assets* assets, const ContentGfxCache* cache)
    : shader(assets->getShader("ui3d")), 
      atlas(assets->getAtlas("blocks")), 
      cache(cache) {
    batch = std::make_unique<Batch3D>(1024);
}

BlocksPreview::~BlocksPreview() {
}

void BlocksPreview::begin(const Viewport* viewport) {
    this->viewport = viewport;
    shader->use();
    shader->uniformMatrix("u_projview", 
        glm::ortho(0.0f, float(viewport->getWidth()), 
                   0.0f, float(viewport->getHeight()), 
                    -1000.0f, 1000.0f) * 
        glm::lookAt(glm::vec3(2, 2, 2), glm::vec3(0.0f), glm::vec3(0, 1, 0)));
    atlas->getTexture()->bind();
}

/* Draw one block preview at given screen position */
void BlocksPreview::draw(const Block* def, int x, int y, int size, glm::vec4 tint) {
    uint width = viewport->getWidth();
    uint height = viewport->getHeight();

    y = height - y - 1 - 35 /* magic garbage */;
    x += 2;

    glm::vec3 offset (x/float(width) * 2, y/float(height) * 2, 0.0f);
    shader->uniformMatrix("u_apply", glm::translate(glm::mat4(1.0f), offset));
    blockid_t id = def->rt.id;
    const UVRegion texfaces[6]{ cache->getRegion(id, 0), cache->getRegion(id, 1),
                                cache->getRegion(id, 2), cache->getRegion(id, 3),
                                cache->getRegion(id, 4), cache->getRegion(id, 5)};

    switch (def->model) {
        case BlockModel::none:
            // something went wrong...
            break;
        case BlockModel::block:
            batch->blockCube(glm::vec3(size * 0.63f), texfaces, tint, !def->rt.emissive);
            break;
        case BlockModel::aabb:
            batch->blockCube(def->hitbox.size() * glm::vec3(size * 0.63f), 
                             texfaces, tint, !def->rt.emissive);
            break;
        case BlockModel::custom:
        case BlockModel::xsprite: {
            glm::vec3 right = glm::normalize(glm::vec3(1.f, 0.f, -1.f));
            batch->sprite(right*float(size)*0.43f+glm::vec3(0, size*0.4f, 0), 
                          glm::vec3(0.f, 1.f, 0.f), 
                          right, 
                          size*0.5f, size*0.6f, 
                          texfaces[0], 
                          tint);
            break;
        }
    }
    
    batch->flush();
}
