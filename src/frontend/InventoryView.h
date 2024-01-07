#ifndef FRONTEND_INVENTORY_VIEW_H_
#define FRONTEND_INVENTORY_VIEW_H_

#include <memory>
#include <vector>
#include <functional>
#include <glm/glm.hpp>

#include "../typedefs.h"

class Assets;
class GfxContext;
class ContentIndices;
class BlocksPreview;
class ContentGfxCache;

typedef std::function<void(blockid_t)> slotconsumer;

class InventoryView {
    const Assets* assets;
    const ContentIndices* indices;
    const ContentGfxCache* const cache;
    std::vector<blockid_t> blocks;
    std::unique_ptr<BlocksPreview> blocksPreview;
    slotconsumer consumer = nullptr;

    int scroll = 0;
    int columns;
    uint iconSize = 48;
    uint interval = 4;
    glm::ivec2 padding {interval, interval};
    glm::ivec2 position {0, 0};
public:
    InventoryView(
        int columns,
        const Assets* assets,
        const ContentIndices* indices, 
        const ContentGfxCache* cache,
        std::vector<blockid_t> blocks);

    void setPosition(int x, int y);
    void actAndDraw(const GfxContext* ctx);
    int getWidth() const;
    int getHeight() const;
    void setSlotConsumer(slotconsumer consumer);
};

#endif // FRONTEND_INVENTORY_VIEW_H_
