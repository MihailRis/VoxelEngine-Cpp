#pragma once

#include "typedefs.hpp"

#include <memory>

class Content;
class Assets;
struct UVRegion;

class ContentGfxCache {
    const Content* content;
    // array of block sides uv regions (6 per block)
    std::unique_ptr<UVRegion[]> sideregions;
public:
    ContentGfxCache(const Content* content, Assets* assets);
    ~ContentGfxCache();

    inline const UVRegion& getRegion(blockid_t id, int side) const {
        return sideregions[id * 6 + side];
    }
    
    const Content* getContent() const;
};
