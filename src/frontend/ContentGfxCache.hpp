#pragma once

#include "typedefs.hpp"

#include <memory>
#include <stdexcept>
#include <unordered_map>

#include "maths/UVRegion.hpp"
#include "graphics/commons/Model.hpp"

class Content;
class Assets;
class Atlas;
class Block;
struct UVRegion;
struct GraphicsSettings;

class ContentGfxCache {
    const Content& content;
    const Assets& assets;
    const GraphicsSettings& settings;

    // array of block sides uv regions (6 per block)
    std::unique_ptr<UVRegion[]> sideregions;
    std::unordered_map<blockid_t, model::Model> models;
public:
    ContentGfxCache(
        const Content& content,
        const Assets& assets,
        const GraphicsSettings& settings
    );
    ~ContentGfxCache();

    inline const UVRegion& getRegion(blockid_t id, int side) const {
        return sideregions[id * 6 + side];
    }

    const model::Model& getModel(blockid_t id) const;

    void refresh(const Block& block, const Atlas& atlas);

    void refresh();
};
