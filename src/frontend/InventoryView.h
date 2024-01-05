#ifndef FRONTEND_INVENTORY_VIEW_H_
#define FRONTEND_INVENTORY_VIEW_H_

#include <vector>

#include "../typedefs.h"

class Player;
class Assets;
class GfxContext;
class ContentIndices;
class BlocksPreview;
class ContentGfxCache;

class InventoryView {
    Player* player;
    const Assets* assets;
    const ContentIndices* indices;
    const ContentGfxCache* const cache;
    std::vector<blockid_t> blocks;
    BlocksPreview* blocksPreview;

    int inventoryScroll = 0;
    int invColumns;
    uint iconSize = 48;
    uint interval = 4;
    int padX = interval;
	int padY = interval;
    int invX = 0;
    int invY = 0;
public:
    InventoryView(
        int columns,
        Player* player,
        const Assets* assets,
        const ContentIndices* indices, 
        const ContentGfxCache* cache,
        std::vector<blockid_t> blocks);
    void setPosition(int x, int y);
    void actAndDraw(const GfxContext* ctx);
    int getWidth() const;
    int getHeight() const;
};

#endif // FRONTEND_INVENTORY_VIEW_H_
