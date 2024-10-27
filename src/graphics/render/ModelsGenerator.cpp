#include "ModelsGenerator.hpp"

#include "assets/Assets.hpp"
#include "items/ItemDef.hpp"
#include "voxels/Block.hpp"
#include "content/Content.hpp"
#include "debug/Logger.hpp"

static debug::Logger logger("models-generator");

static void configure_textures(
    model::Model& model,
    const Block& blockDef,
    const Assets& assets
) {
    for (auto& mesh : model.meshes) {
        auto& texture = mesh.texture;
        if (texture.empty() || texture.at(0) != '$') {
            continue;
        }
        try {
            int index = std::stoi(texture.substr(1));
            texture = "blocks:"+blockDef.textureFaces.at(index);
        } catch (const std::invalid_argument& err) {
        } catch (const std::runtime_error& err) {
            logger.error() << err.what();
        }
    }
}

static model::Model create_flat_model(
    const std::string& texture, const Assets& assets
) {
    auto model = assets.require<model::Model>("drop-item");
    for (auto& mesh : model.meshes) {
        if (mesh.texture == "$0") {
            mesh.texture = texture;
        }
    }
    return model;
}

model::Model ModelsGenerator::generate(
    const ItemDef& def, const Content& content, const Assets& assets
) {
    if (def.iconType == ItemIconType::BLOCK) {
        auto model = assets.require<model::Model>("block");
        const auto& blockDef = content.blocks.require(def.icon);
        if (blockDef.model == BlockModel::xsprite) {
            return create_flat_model(
                "blocks:" + blockDef.textureFaces.at(0), assets
            );
        }
        for (auto& mesh : model.meshes) {
            switch (blockDef.model) {
                case BlockModel::aabb: {
                    glm::vec3 size = blockDef.hitboxes.at(0).size();
                    float m = glm::max(size.x, glm::max(size.y, size.z));
                    m = glm::min(1.0f, m);
                    mesh.scale(size / m);
                    break;
                } default:
                    break;
            }
            mesh.scale(glm::vec3(0.3f));
        }
        configure_textures(model, blockDef, assets);
        return model;
    } else if (def.iconType == ItemIconType::SPRITE) {
        return create_flat_model(def.icon, assets);
    } else {
        return model::Model();
    }
}
