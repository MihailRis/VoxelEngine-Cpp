#include "InventoryView.h"

#include <glm/glm.hpp>

#include "BlocksPreview.h"
#include "LevelFrontend.h"
#include "../window/Events.h"
#include "../assets/Assets.h"
#include "../graphics/Atlas.h"
#include "../graphics/Shader.h"
#include "../graphics/Batch2D.h"
#include "../graphics/GfxContext.h"
#include "../content/Content.h"
#include "../items/ItemDef.h"
#include "../maths/voxmaths.h"
#include "../objects/Player.h"
#include "../voxels/Block.h"

ItemStorage::ItemStorage(size_t slots) {
    items.resize(slots);
}

ItemStorage::ItemStorage(const std::vector<itemid_t>& items) :
    items(items)
{
}

ItemStorage::~ItemStorage()
{
}

void ItemStorage::setItems(const std::vector<itemid_t>& items) {
    this->items = items;
}

void ItemStorage::setItem(size_t slot, itemid_t item) {
    if (slot >= items.size()) return;
    items[slot] = item;
}

itemid_t ItemStorage::viewItem(size_t slot) const {
    if (slot >= items.size()) return 0;
    return items[slot];
}

void ItemStorage::clearSlot(size_t slot) {
    if (slot >= items.size()) return;
    items[slot] = 0;
}

size_t ItemStorage::getSize() {
    return items.size();
}

InventoryView::InventoryView(
            int columns,
            const Content* content,
            LevelFrontend* frontend,
            const std::vector<itemid_t>& items) 
            : ItemStorage(items),
              content(content),
              indices(content->indices), 
              frontend(frontend),
              columns(columns)
{
}

InventoryView::~InventoryView() {
}

void InventoryView::setPosition(int x, int y) {
    position.x = x;
    position.y = y;
}

int InventoryView::getWidth() const {
    return columns * iconSize + (columns-1) * interval + padding.x * 2;
}

int InventoryView::getHeight() const {
    uint inv_rows = ceildiv(items.size(), columns);
    return inv_rows * iconSize + (inv_rows-1) * interval + padding.y * 2;
}

void InventoryView::setSlotConsumer(slotconsumer consumer) {
    this->consumer = consumer;
}

void InventoryView::setHoverSlot(int slot) {
    hoverSlot = slot;
}

void InventoryView::actAndDraw(const GfxContext* ctx) {
    Assets* assets = frontend->getAssets();
    Shader* uiShader = assets->getShader("ui");

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
	batch->color = glm::vec4(0.0f, 0.0f, 0.0f, 0.5f);
	batch->rect(position.x, position.y, inv_w, inv_h);
	batch->render();

    if (hoverSlot > -1) {
        int x = xs + (iconSize + interval) * (hoverSlot % columns);
        int y = ys + (iconSize + interval) * (hoverSlot / columns) - scroll;
        batch->color = glm::vec4(1.f, 1.f, 1.f, 0.5f);
        batch->rect(x, y, iconSize, iconSize);
        batch->render();
    }

	// blocks & items
    if (Events::scroll) {
        scroll -= Events::scroll * (iconSize+interval);
    }
    scroll = std::min(scroll, int(inv_h-viewport.getHeight()));
    scroll = std::max(scroll, 0);

    auto blocksPreview = frontend->getBlocksPreview();
    // todo: optimize
	{
		Window::clearDepth();
		GfxContext subctx = ctx->sub();
		subctx.depthTest(true);
		subctx.cullFace(true);
        uint index = 0;
		for (uint i = 0; i < items.size(); i++) {
            ItemDef* item = indices->getItemDef(items[i]);
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
                        consumer(items[i]);
                    }
				}
			} else {
				tint = glm::vec4(1.0f);
			}
            switch (item->iconType) {
                case item_icon_type::none:
                    break;
                case item_icon_type::block: {
                    Block* cblock = content->requireBlock(item->icon);
                    blocksPreview->begin(&ctx->getViewport());
                    blocksPreview->draw(cblock, x, y, iconSize, tint);
                    break;
                }
                case item_icon_type::sprite: {
                    batch->begin();
                    uiShader->use();
                    size_t index = item->icon.find(':');
                    std::string name = item->icon.substr(index+1);
                    UVRegion region(0.0f, 0.0, 1.0f, 1.0f);
                    if (index == std::string::npos) {
                        batch->texture(assets->getTexture(name));
                    } else {
                        std::string atlasname = item->icon.substr(0, index);
                        Atlas* atlas = assets->getAtlas(atlasname);
                        if (atlas && atlas->has(name)) {
                            region = atlas->get(name);
                            batch->texture(atlas->getTexture());
                        }
                    }
                    batch->rect(x, y, 48, 48, 0, 0, 0, region, false, true, glm::vec4(1.0f));
                    batch->render();
                    break;
                }
            }
            index++;
		}
	}
	uiShader->use();
}
