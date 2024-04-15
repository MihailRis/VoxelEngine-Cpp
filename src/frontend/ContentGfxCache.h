#ifndef FRONTEND_BLOCKS_GFX_CACHE_H_
#define FRONTEND_BLOCKS_GFX_CACHE_H_

#include <memory>
#include <string>
#include <unordered_map>
#include "../typedefs.h"

class Content;
class Assets;
class UiDocument;
struct UVRegion;

using uidocuments_map = std::unordered_map<std::string, std::shared_ptr<UiDocument>>;

class ContentGfxCache {
    const Content* content;
    // array of block sides uv regions (6 per block)
    std::unique_ptr<UVRegion[]> sideregions;
    // all loaded layouts
    uidocuments_map layouts;
public:
    ContentGfxCache(const Content* content, Assets* assets);
    ~ContentGfxCache();

    inline const UVRegion& getRegion(blockid_t id, int side) const {
        return sideregions[id * 6 + side];
    }

    std::shared_ptr<UiDocument> getLayout(const std::string& id);
    
    const Content* getContent() const;
};

#endif // FRONTEND_BLOCKS_GFX_CACHE_H_
