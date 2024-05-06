#include "ContentGfxCache.hpp"

#include "UiDocument.hpp"

#include "../assets/Assets.hpp"
#include "../content/Content.hpp"
#include "../content/ContentPack.hpp"
#include "../core_defs.hpp"
#include "../graphics/core/Atlas.hpp"
#include "../maths/UVRegion.hpp"
#include "../voxels/Block.hpp"

#include <string>

ContentGfxCache::ContentGfxCache(const Content* content, Assets* assets) : content(content) {
    auto indices = content->getIndices();
    sideregions = std::make_unique<UVRegion[]>(indices->countBlockDefs() * 6);
    Atlas* atlas = assets->getAtlas("blocks");
    
    for (uint i = 0; i < indices->countBlockDefs(); i++) {
        Block* def = indices->getBlockDef(i);
        for (uint side = 0; side < 6; side++) {
            const std::string& tex = def->textureFaces[side];
            if (atlas->has(tex)) {
                sideregions[i * 6 + side] = atlas->get(tex);
            } else if (atlas->has(TEXTURE_NOTFOUND)) {
                sideregions[i * 6 + side] = atlas->get(TEXTURE_NOTFOUND);
            }
        }
        for (uint side = 0; side < def->modelTextures.size(); side++) {
            const std::string& tex = def->modelTextures[side];
            if (atlas->has(tex)) {
                def->modelUVs.push_back(atlas->get(tex));
            } else if (atlas->has(TEXTURE_NOTFOUND)) {
                def->modelUVs.push_back(atlas->get(TEXTURE_NOTFOUND));
            }
        }
    }
}

ContentGfxCache::~ContentGfxCache() {
}

const Content* ContentGfxCache::getContent() const {
    return content;
}
