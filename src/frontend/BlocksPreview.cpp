#include "BlocksPreview.h"

#include <glm/ext.hpp>

#include "../assets/Assets.h"
#include "../graphics/Viewport.h"
#include "../graphics/Shader.h"
#include "../graphics/Texture.h"
#include "../graphics/Atlas.h"
#include "../graphics/Batch3D.h"
#include "../graphics/Framebuffer.h"
#include "../graphics/GfxContext.h"
#include "../window/Window.h"
#include "../window/Camera.h"
#include "../voxels/Block.h"
#include "../content/Content.h"
#include "../constants.h"
#include "ContentGfxCache.h"

ImageData* BlocksPreview::draw(
    const ContentGfxCache* cache,
    Framebuffer* fbo,
    Batch3D* batch,
    const Block* def, 
    int size
){
    Window::clear();
    blockid_t id = def->rt.id;
    const UVRegion texfaces[6]{cache->getRegion(id, 0), cache->getRegion(id, 1),
                               cache->getRegion(id, 2), cache->getRegion(id, 3),
                               cache->getRegion(id, 4), cache->getRegion(id, 5)};

    switch (def->model) {
        case BlockModel::none:
            // something went wrong...
            break;
        case BlockModel::block:
            batch->blockCube(glm::vec3(size * 0.63f), texfaces, 
                             glm::vec4(1.0f), !def->rt.emissive);
            break;
        case BlockModel::aabb:
            {
                glm::vec3 hitbox = glm::vec3();
                for (const auto& box : def->hitboxes)
                    hitbox = glm::max(hitbox, box.size());
                batch->blockCube(hitbox * glm::vec3(size * 0.63f), 
                                 texfaces, glm::vec4(1.0f), !def->rt.emissive);
            }
            break;
        case BlockModel::custom:
        case BlockModel::xsprite: {
            glm::vec3 right = glm::normalize(glm::vec3(1.f, 0.f, -1.f));
            batch->sprite(right*float(size)*0.43f+glm::vec3(0, size*0.4f, 0), 
                          glm::vec3(0.f, 1.f, 0.f), 
                          right, 
                          size*0.5f, size*0.6f, 
                          texfaces[0], 
                          glm::vec4(1.0f));
            break;
        }
    }
    batch->flush();
    return fbo->texture->readData();
}

std::unique_ptr<Atlas> BlocksPreview::build(
    const ContentGfxCache* cache,
    Assets* assets, 
    const Content* content
) {
    auto indices = content->getIndices();
    size_t count = indices->countBlockDefs();
    size_t iconSize = ITEM_ICON_SIZE;

    Shader* shader = assets->getShader("ui3d");
    Atlas* atlas = assets->getAtlas("blocks");

    Viewport viewport(iconSize, iconSize);
    GfxContext pctx(nullptr, viewport, nullptr);
    GfxContext ctx = pctx.sub();
    ctx.cullFace(true);
    ctx.depthTest(true);

    Framebuffer fbo(iconSize, iconSize, true);
    Batch3D batch(1024);
    batch.begin();

    shader->use();
    shader->uniformMatrix("u_projview",

    glm::ortho(0.0f, float(iconSize), 0.0f, float(iconSize), 
                -100.0f, 100.0f) * 
        glm::lookAt(glm::vec3(2, 2, 2), 
                    glm::vec3(0.0f), 
                    glm::vec3(0, 1, 0)));

    AtlasBuilder builder;
    Window::viewport(0, 0, iconSize, iconSize);
    Window::setBgColor(glm::vec4(0.0f));
    
    fbo.bind();
    for (size_t i = 0; i < count; i++) {
        auto def = indices->getBlockDef(i);

        glm::vec3 offset(0.1f, 0.5f, 0.1f);
        if (def->model == BlockModel::aabb) {
            glm::vec3 size = glm::vec3(0, 0, 0);
            for (const auto& box : def->hitboxes)
                size = glm::max(size, box.size());
            offset.y += (1.0f - size).y * 0.5f;
        }
        atlas->getTexture()->bind();
        shader->uniformMatrix("u_apply", glm::translate(glm::mat4(1.0f), offset));

        builder.add(def->name, draw(cache, &fbo, &batch, def, iconSize));
    }
    fbo.unbind();

    Window::viewport(0, 0, Window::width, Window::height);
    return std::unique_ptr<Atlas>(builder.build(2));
}
