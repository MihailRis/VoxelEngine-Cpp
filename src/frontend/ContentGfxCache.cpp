#include "ContentGfxCache.h"

#include <string>

#include "../assets/Assets.h"
#include "../content/Content.h"
#include "../graphics/Atlas.h"
#include "../voxels/Block.h"

ContentGfxCache::ContentGfxCache(const Content* content, Assets* assets) {
    auto indices = content->getIndices();
    sideregions = new UVRegion[indices->countBlockDefs() * 6];
	Atlas* atlas = assets->getAtlas("blocks");
	
	for (uint i = 0; i < indices->countBlockDefs(); i++) {
		Block* def = indices->getBlockDef(i);
		for (uint side = 0; side < 6; side++) {
			std::string tex = def->textureFaces[side];
			if (atlas->has(tex)) {
				sideregions[i * 6 + side] = atlas->get(tex);
			} else {
				if (atlas->has("notfound"))
					sideregions[i * 6 + side] = atlas->get("notfound");
			}
		}
		for (uint side = 0; side < def->modelTextures.size(); side++)
		{
			std::string tex = def->modelTextures[side];
			if (atlas->has(tex)) {
				def->modelUVs.push_back(atlas->get(tex));
			} else {
				if (atlas->has("notfound"))
					def->modelUVs.push_back(atlas->get("notfound"));
			}
		}
    }
}

ContentGfxCache::~ContentGfxCache() {
	delete[] sideregions;
}
