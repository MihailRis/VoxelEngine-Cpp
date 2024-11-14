#include "ContentGfxCache.hpp"

#include <string>

#include "UiDocument.hpp"
#include "assets/Assets.hpp"
#include "content/Content.hpp"
#include "content/ContentPack.hpp"
#include "core_defs.hpp"
#include "graphics/core/Atlas.hpp"
#include "maths/UVRegion.hpp"
#include "voxels/Block.hpp"

ContentGfxCache::ContentGfxCache(const Content* content, const Assets& assets)
    : content(content) {
    auto indices = content->getIndices();
    sideregions = std::make_unique<UVRegion[]>(indices->blocks.count() * 6);
    const auto& atlas = assets.require<Atlas>("blocks");

    const auto& blocks = indices->blocks.getIterable();
    for (blockid_t i = 0; i < blocks.size(); i++) {
        auto def = blocks[i];
        for (uint side = 0; side < 6; side++) {
            const std::string& tex = def->textureFaces[side];
            if (atlas.has(tex)) {
                sideregions[i * 6 + side] = atlas.get(tex);
            } else if (atlas.has(TEXTURE_NOTFOUND)) {
                sideregions[i * 6 + side] = atlas.get(TEXTURE_NOTFOUND);
            }
        }
        if (def->model == BlockModel::custom) {
            auto model = assets.require<model::Model>(def->modelName);
            // temporary dirty fix tbh
            if (def->modelName.find(':') == std::string::npos) {
                for (auto& mesh : model.meshes) {
                    size_t pos = mesh.texture.find(':');
                    if (pos == std::string::npos) {
                        continue;
                    }
                    if (auto region = atlas.getIf(mesh.texture.substr(pos+1))) {
                        for (auto& vertex : mesh.vertices) {
                            vertex.uv = region->apply(vertex.uv);
                        }
                    }
                }
            }
            models[def->rt.id] = std::move(model);
        }
    }
}

ContentGfxCache::~ContentGfxCache() = default;

const Content* ContentGfxCache::getContent() const {
    return content;
}

const model::Model& ContentGfxCache::getModel(blockid_t id) const {
    const auto& found = models.find(id);
    if (found == models.end()) {
        throw std::runtime_error("model not found");
    }
    return found->second;
}
