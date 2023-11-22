#ifndef FRONTEND_BLOCKS_GFX_CACHE_H_
#define FRONTEND_BLOCKS_GFX_CACHE_H_

#include "../graphics/UVRegion.h"
#include "../typedefs.h"

class Content;
class Assets;

class ContentGfxCache {
    // array of block sides uv regions (6 per block)
    UVRegion* sideregions;
public:
    ContentGfxCache(const Content* content, Assets* assets);

    inline const UVRegion& getRegion(blockid_t id, int side) const {
        return sideregions[id * 6 + side];
    }
};

#endif // FRONTEND_BLOCKS_GFX_CACHE_H_