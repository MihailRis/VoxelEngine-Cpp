#include "InventoryView.h"

#include <glm/glm.hpp>

#include "BlocksPreview.h"
#include "../window/Events.h"
#include "../assets/Assets.h"
#include "../graphics/Shader.h"
#include "../graphics/Batch2D.h"
#include "../graphics/GfxContext.h"
#include "../content/Content.h"
#include "../maths/voxmaths.h"
#include "../objects/Player.h"
#include "../voxels/Block.h"

InventoryView::InventoryView(
            int columns,
            Player* player,
            const Assets* assets,
            const ContentIndices* indices,
            const ContentGfxCache* cache, 
            std::vector<blockid_t> blocks)
            : player(player),
              assets(assets), 
              indices(indices), 
              cache(cache), 
              blocks(blocks),
              invColumns(columns) {
    blocksPreview = new BlocksPreview(assets->getShader("ui3d"),
                                      assets->getAtlas("blocks"),
                                      cache);
}

void InventoryView::setPosition(int x, int y) {
    this->invX = x;
    this->invY = y;
}

int InventoryView::getWidth() const {
    return invColumns * iconSize + (invColumns-1) * interval + padX * 2;
}

int InventoryView::getHeight() const {
    uint inv_rows = ceildiv(blocks.size(), invColumns);
    return inv_rows * iconSize + (inv_rows-1) * interval + padY * 2;
}

void InventoryView::actAndDraw(const GfxContext* ctx) {
    Shader* uiShader = assets->getShader("ui");

    auto viewport = ctx->getViewport();
	uint inv_w = getWidth();
	uint inv_h = getHeight();
	int xs = invX + padX;
	int ys = invY + padY;

	glm::vec4 tint (1.0f);
	int mx = Events::cursor.x;
	int my = Events::cursor.y;

	// background
    auto batch = ctx->getBatch2D();
	batch->texture(nullptr);
	batch->color = glm::vec4(0.0f, 0.0f, 0.0f, 0.5f);
	batch->rect(invX, invY, inv_w, inv_h);
	batch->render();

	// blocks & items
    if (Events::scroll) {
        inventoryScroll -= Events::scroll * (iconSize+interval);
    }
    inventoryScroll = std::min(inventoryScroll, int(inv_h-viewport.getHeight()));
    inventoryScroll = std::max(inventoryScroll, 0);
	blocksPreview->begin(&ctx->getViewport());
	{
		Window::clearDepth();
		GfxContext subctx = ctx->sub();
		subctx.depthTest(true);
		subctx.cullFace(true);
        uint index = 0;
		for (uint i = 0; i < blocks.size(); i++) {
			Block* cblock = indices->getBlockDef(blocks[i]);
			int x = xs + (iconSize+interval) * (index % invColumns);
			int y = ys + (iconSize+interval) * (index / invColumns) - inventoryScroll;
            if (y < -int(iconSize+interval) || y >= int(viewport.getHeight())) {
                index++;
                continue;
            }
			if (mx > x && mx < x + (int)iconSize && my > y && my < y + (int)iconSize) {
				tint.r *= 1.2f;
				tint.g *= 1.2f;
				tint.b *= 1.2f;
				if (Events::jclicked(mousecode::BUTTON_1)) {
					player->chosenBlock = blocks[i];
				}
			} else {
				tint = glm::vec4(1.0f);
			}
			blocksPreview->draw(cblock, x, y, iconSize, tint);
            index++;
		}
	}
	uiShader->use();
}
