#include "ContentGfxCache.h"

#include <string>

#include "../assets/Assets.h"
#include "../content/Content.h"
#include "../graphics/Atlas.h"
#include "../voxels/Block.h"

ContentGfxCache::ContentGfxCache(const Content* content, Assets* assets) {
    const ContentIndices* contentIds = content->indices;
    sideregions = new UVRegion[contentIds->countBlockDefs() * 6];
	Atlas* atlas = assets->getAtlas("blocks");
	
	for (uint i = 0; i < contentIds->countBlockDefs(); i++) {
		Block* def = contentIds->getBlockDef(i);
		for (uint side = 0; side < 6; side++) {
			std::string tex = def->textureFaces[side];
			if (atlas->has(tex)) {
				sideregions[i * 6 + side] = atlas->get(tex);
			} else {
				if (atlas->has("notfound"))
					sideregions[i * 6 + side] = atlas->get("notfound");
			}
		}
		for (uint side = 0; side < def->textureMoreFaces.size(); side++)
		{
			std::string tex = def->textureMoreFaces[side];
			if (atlas->has(tex)) {
				def->customfacesExtraUVs.push_back(atlas->get(tex));
			} else {
				if (atlas->has("notfound"))
					def->customfacesExtraUVs.push_back(atlas->get("notfound"));
			}
		}
    }
}

ContentGfxCache::~ContentGfxCache() {
	delete[] sideregions;
}
