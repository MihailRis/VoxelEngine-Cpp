#include "InventoryView.h"

#include <glm/glm.hpp>
#include <utility>

#include "BlocksPreview.h"
#include "../window/Events.h"
#include "../assets/Assets.h"
#include "../graphics-common/IShader.h"
#include "../graphics/GfxContext.h"
#include "../content/Content.h"
#include "../maths/voxmaths.h"
#include "../objects/Player.h"

InventoryView::InventoryView(
            int columns,
            const Assets* assets,
            const ContentIndices* indices,
            const ContentGfxCache* cache, 
            std::vector<blockid_t> blocks)
            : assets(assets), 
              indices(indices), 
              cache(cache), 
              blocks(std::move(blocks)),
              columns(columns) {
    blocksPreview = std::make_unique<BlocksPreview>(assets->getShader("ui3d"),
                                      assets->getAtlas("blocks"),
                                      cache);
}

void InventoryView::setPosition(int x, int y) {
    position.x = x;
    position.y = y;
}

int InventoryView::getWidth() const {
    return columns * iconSize + (columns-1) * interval + padding.x * 2;
}

int InventoryView::getHeight() const {
    uint inv_rows = ceildiv(blocks.size(), columns);
    return inv_rows * iconSize + (inv_rows-1) * interval + padding.y * 2;
}

void InventoryView::setSlotConsumer(slotconsumer consumer) {
    this->consumer = consumer;
}

void InventoryView::actAndDraw(const GfxContext* ctx) {
    IShader* uiShader = assets->getShader("ui");

    auto viewport = ctx->getViewport();
	uint inv_w = getWidth();
	uint inv_h = getHeight();
	int xs = position.x + padding.x;
	int ys = position.y + padding.y;

	glm::vec4 tint (1.0f);
	int mx = Events::cursor.x;
	int my = Events::cursor.y;

	// background
    auto batch = ctx->getBatch2D();
	batch->texture(nullptr);
	batch->begin();
	batch->setColor(glm::vec4(0.0f, 0.0f, 0.0f, 0.5f));
	batch->rect(position.x, position.y, inv_w, inv_h);
	batch->render();
	batch->end();

	// blocks & items
    if (Events::scroll) {
        scroll -= Events::scroll * (iconSize+interval);
    }
    scroll = std::min(scroll, int(inv_h-viewport.getHeight()));
    scroll = std::max(scroll, 0);
	blocksPreview->begin(&ctx->getViewport());
	{
		Window::clearDepth();
		GfxContext subctx = ctx->sub();
		subctx.depthTest(true);
		subctx.cullFace(true);
        uint index = 0;
		for (uint i = 0; i < blocks.size(); i++) {
			Block* cblock = indices->getBlockDef(blocks[i]);
			int x = xs + (iconSize+interval) * (index % columns);
			int y = ys + (iconSize+interval) * (index / columns) - scroll;
            if (y < -int(iconSize+interval) || y >= int(viewport.getHeight())) {
                index++;
                continue;
            }
			if (mx > x && mx < x + (int)iconSize && my > y && my < y + (int)iconSize) {
				tint.r *= 1.2f;
				tint.g *= 1.2f;
				tint.b *= 1.2f;
				if (Events::jclicked(mousecode::BUTTON_1)) {
                    if (consumer) {
                        consumer(blocks[i]);
                    }
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
