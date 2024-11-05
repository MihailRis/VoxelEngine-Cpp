#pragma once

#include "typedefs.hpp"

#include <memory>
#include <stdexcept>
#include <unordered_map>

#include "graphics/commons/Model.hpp"

class Content;
class Assets;
struct UVRegion;

namespace model {
    struct Model;
}

class ContentGfxCache {
    const Content* content;
    // array of block sides uv regions (6 per block)
    std::unique_ptr<UVRegion[]> sideregions;
    std::unordered_map<blockid_t, model::Model> models;
public:
    ContentGfxCache(const Content* content, Assets* assets);
    ~ContentGfxCache();

    inline const UVRegion& getRegion(blockid_t id, int side) const {
        return sideregions[id * 6 + side];
    }

    const model::Model& getModel(blockid_t id) const;
    
    const Content* getContent() const;
};
