#include "ModelsGenerator.hpp"

#include "assets/Assets.hpp"
#include "assets/assets_util.hpp"
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

static inline UVRegion get_region_for(
    const std::string& texture, const Assets& assets
) {
    auto texreg = util::get_texture_region(assets, "blocks:" + texture, "");
    return texreg.region;
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
        } else if (blockDef.model == BlockModel::custom) {
            model = model::Model();
            for (size_t i = 0; i < blockDef.modelBoxes.size(); i++) {
                auto& mesh =
                    model.addMesh("blocks:" + blockDef.modelTextures[i * 6]);
                mesh.lighting = !def.rt.emissive;
                const UVRegion (&boxtexfaces)[6] = {
                    get_region_for(blockDef.modelTextures[i * 6], assets),
                    get_region_for(blockDef.modelTextures[i * 6 + 1], assets),
                    get_region_for(blockDef.modelTextures[i * 6 + 2], assets),
                    get_region_for(blockDef.modelTextures[i * 6 + 3], assets),
                    get_region_for(blockDef.modelTextures[i * 6 + 4], assets),
                    get_region_for(blockDef.modelTextures[i * 6 + 5], assets)
                };
                mesh.addBox(
                    blockDef.modelBoxes[i].center(),
                    blockDef.modelBoxes[i].size()*0.5f, boxtexfaces
                );
            }
            const auto& points = blockDef.modelExtraPoints;
            glm::vec3 poff = glm::vec3(0.0f, 0.0f, 1.0f);
            glm::vec3 norm {0, 1, 0};
            for (size_t i = 0; i < blockDef.modelExtraPoints.size() / 4; i++) {
                auto texture =
                    "blocks:" +
                    blockDef.modelTextures[blockDef.modelBoxes.size() * 6 + i];
                    
                auto& mesh = model.addMesh(texture);
                mesh.lighting = !def.rt.emissive;

                auto reg = get_region_for(texture, assets);
                mesh.vertices.push_back(
                    {points[i * 4 + 0] - poff, glm::vec2(reg.u1, reg.v1), norm}
                );
                mesh.vertices.push_back(
                    {points[i * 4 + 1] - poff, glm::vec2(reg.u2, reg.v1), norm}
                );
                mesh.vertices.push_back(
                    {points[i * 4 + 2] - poff, glm::vec2(reg.u2, reg.v2), norm}
                );
                mesh.vertices.push_back(
                    {points[i * 4 + 3] - poff, glm::vec2(reg.u1, reg.v1), norm}
                );
                mesh.vertices.push_back(
                    {points[i * 4 + 4] - poff, glm::vec2(reg.u2, reg.v2), norm}
                );
                mesh.vertices.push_back(
                    {points[i * 4 + 0] - poff, glm::vec2(reg.u1, reg.v2), norm}
                );
            }
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
