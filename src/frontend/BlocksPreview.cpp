#include "BlocksPreview.h"

#include <glm/ext.hpp>

#include "../graphics/Viewport.h"
#include "../graphics/Shader.h"
#include "../graphics/Texture.h"
#include "../graphics/Atlas.h"
#include "../graphics/Batch3D.h"
#include "../window/Camera.h"
#include "../voxels/Block.h"
#include "ContentGfxCache.h"

using glm::vec4;
using glm::vec3;

BlocksPreview::BlocksPreview(Shader* shader, 
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
    shader->uniformMatrix("u_projview", 
        glm::ortho(0.0f, float(viewport->getWidth()), 
                   0.0f, float(viewport->getHeight()), 
                    -1000.0f, 1000.0f) * 
        glm::lookAt(vec3(2, 2, 2), vec3(0.0f), vec3(0, 1, 0)));
    atlas->getTexture()->bind();
}

/* Draw one block preview at given screen position */
void BlocksPreview::draw(const Block* def, int x, int y, int size, vec4 tint) {
    uint width = viewport->getWidth();
    uint height = viewport->getHeight();

    y = height - y - 1;
    x += 2;
    y -= 35;
    shader->uniformMatrix("u_apply", glm::translate(glm::mat4(1.0f), vec3(x/float(width) * 2, y/float(height) * 2, 0.0f)));
    blockid_t id = def->rt.id;
    const UVRegion texfaces[6]{ cache->getRegion(id, 0), cache->getRegion(id, 1),
                                cache->getRegion(id, 2), cache->getRegion(id, 3),
                                cache->getRegion(id, 4), cache->getRegion(id, 5)};

    switch (def->model) {
        case BlockModel::none:
            // something went wrong...
            break;
        case BlockModel::block:
            batch->blockCube(vec3(size * 0.63f), texfaces, tint, !def->rt.emissive);
            break;
        case BlockModel::aabb:
            batch->blockCube(def->hitbox.size() * vec3(size * 0.63f), texfaces, tint, !def->rt.emissive);
            break;
        case BlockModel::xsprite: {
            //batch->xSprite(size, size, texfaces[0], tint, !def->rt.emissive);
            vec3 right = glm::normalize(vec3(1.f, 0.f, -1.f));
            batch->sprite(right*float(size)*0.43f+vec3(0, size*0.4f, 0), vec3(0.f, 1.f, 0.f), right, size*0.5f, size*0.6f, texfaces[0], tint);
            break;
        }
    }
    
    batch->flush();
}
