#include "InventoryView.hpp"

#include "assets/Assets.hpp"
#include "assets/assets_util.hpp"
#include "content/Content.hpp"
#include "frontend/LevelFrontend.hpp"
#include "frontend/locale.hpp"
#include "items/Inventories.hpp"
#include "items/Inventory.hpp"
#include "items/ItemDef.hpp"
#include "items/ItemStack.hpp"
#include "logic/scripting/scripting.hpp"
#include "maths/voxmaths.hpp"
#include "objects/Player.hpp"
#include "util/stringutil.hpp"
#include "voxels/Block.hpp"
#include "window/Events.hpp"
#include "window/input.hpp"
#include "world/Level.hpp"
#include "graphics/core/Atlas.hpp"
#include "graphics/core/Batch2D.hpp"
#include "graphics/core/Font.hpp"
#include "graphics/core/DrawContext.hpp"
#include "graphics/core/Shader.hpp"
#include "graphics/core/Texture.hpp"
#include "graphics/render/BlocksPreview.hpp"
#include "graphics/ui/GUI.hpp"

#include <glm/glm.hpp>
#include <utility>

using namespace gui;

SlotLayout::SlotLayout(
    int index,
    glm::vec2 position,
    bool background,
    bool itemSource,
    slotcallback updateFunc,
    slotcallback shareFunc,
    slotcallback rightClick
) : index(index),
    position(position),
    background(background),
    itemSource(itemSource),
    updateFunc(std::move(updateFunc)),
    shareFunc(std::move(shareFunc)),
    rightClick(std::move(rightClick)) {}

InventoryBuilder::InventoryBuilder() {
    view = std::make_shared<InventoryView>();
}

void InventoryBuilder::addGrid(
    int cols, int count, 
    glm::vec2 pos, 
    int padding,
    bool addpanel,
    const SlotLayout& slotLayout
) {
    const int slotSize = InventoryView::SLOT_SIZE;
    const int interval = InventoryView::SLOT_INTERVAL;

    int rows = ceildiv(count, cols);

    uint width =  cols * (slotSize + interval) - interval + padding*2;
    uint height = rows * (slotSize + interval) - interval + padding*2;
    
    glm::vec2 vsize = view->getSize();
    if (pos.x + width > vsize.x) {
        vsize.x = pos.x + width;
    }
    if (pos.y + height > vsize.y) {
        vsize.y = pos.y + height;
    }
    view->setSize(vsize);

    if (addpanel) {
        auto panel = std::make_shared<gui::Container>(glm::vec2(width, height));
        view->setColor(glm::vec4(0.122f, 0.122f, 0.122f, 0.878f));
        view->add(panel, pos);
    }

    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < cols; col++) {
            if (row * cols + col >= count) {
                break;
            }
            glm::vec2 position (
                col * (slotSize + interval) + padding,
                row * (slotSize + interval) + padding
            );
            auto builtSlot = slotLayout;
            builtSlot.index = row * cols + col;
            builtSlot.position = position;
            add(builtSlot);
        }
    }
}

void InventoryBuilder::add(const SlotLayout& layout) {
    view->add(view->addSlot(layout), layout.position);
}

std::shared_ptr<InventoryView> InventoryBuilder::build() {
    return view;
}

SlotView::SlotView(
    SlotLayout layout
) : UINode(glm::vec2(InventoryView::SLOT_SIZE)),
    layout(std::move(layout))
{
    setColor(glm::vec4(0, 0, 0, 0.2f));
    setTooltipDelay(0.05f);
}

void SlotView::draw(const DrawContext* pctx, Assets* assets) {
    if (bound == nullptr) {
        return;
    }
    itemid_t itemid = bound->getItemId();
    if (itemid != prevItem) {
        if (itemid) {
            auto& def = content->getIndices()->items.require(itemid);
            tooltip = util::pascal_case(
                langs::get(util::str2wstr_utf8(def.caption))
            );
        } else {
            tooltip.clear();
        }
    }
    prevItem = itemid;

    const int slotSize = InventoryView::SLOT_SIZE;

    const ItemStack& stack = *bound;
    glm::vec4 tint(1.0f);
    glm::vec2 pos = calcPos();
    glm::vec4 color = getColor();
    
    if (hover || highlighted) {
        tint *= 1.333f;
        color = glm::vec4(1, 1, 1, 0.2f);
    }

    auto batch = pctx->getBatch2D();
    batch->setColor(color);
    if (color.a > 0.0) {
        batch->texture(nullptr);
        if (highlighted) {
            batch->rect(pos.x-4, pos.y-4, slotSize+8, slotSize+8);
        } else {
            batch->rect(pos.x, pos.y, slotSize, slotSize);
        }
    }
    
    batch->setColor(glm::vec4(1.0f));

    auto previews = assets->get<Atlas>("block-previews");
    auto indices = content->getIndices();

    auto& item = indices->items.require(stack.getItemId());
    switch (item.iconType) {
        case ItemIconType::NONE:
            break;
        case ItemIconType::BLOCK: {
            const Block& cblock = content->blocks.require(item.icon);
            batch->texture(previews->getTexture());

            UVRegion region = previews->get(cblock.name);
            batch->rect(
                pos.x, pos.y, slotSize, slotSize, 
                0, 0, 0, region, false, true, tint);
            break;
        }
        case ItemIconType::SPRITE: {
            auto textureRegion =
                util::get_texture_region(*assets, item.icon, "blocks:notfound");
            
            batch->texture(textureRegion.texture);
            batch->rect(
                pos.x, pos.y, slotSize, slotSize, 
                0, 0, 0, textureRegion.region, false, true, tint);
            break;
        }
    }

    if (stack.getCount() > 1) {
        auto font = assets->get<Font>("normal");
        std::wstring text = std::to_wstring(stack.getCount());

        int x = pos.x+slotSize-text.length()*8;
        int y = pos.y+slotSize-16;

        batch->setColor({0, 0, 0, 1.0f});
        font->draw(batch, text, x+1, y+1);
        batch->setColor(glm::vec4(1.0f));
        font->draw(batch, text, x, y);
    }
}

void SlotView::setHighlighted(bool flag) {
    highlighted = flag;
}

bool SlotView::isHighlighted() const {
    return highlighted;
}

void SlotView::clicked(gui::GUI* gui, mousecode button) {
    if (bound == nullptr)
        return;

    auto exchangeSlot = std::dynamic_pointer_cast<SlotView>(gui->get(EXCHANGE_SLOT_NAME));
    if (exchangeSlot == nullptr) {
        return;
    }
    ItemStack& grabbed = exchangeSlot->getStack();
    ItemStack& stack = *bound;
    
    if (button == mousecode::BUTTON_1) {
        if (Events::pressed(keycode::LEFT_SHIFT)) {
            if (layout.shareFunc) {
                layout.shareFunc(layout.index, stack);
            }
            if (layout.updateFunc) {
                layout.updateFunc(layout.index, stack);
            }
            return;
        }
        if (!layout.itemSource && stack.accepts(grabbed)) {
            stack.move(grabbed, content->getIndices());
        } else {
            if (layout.itemSource) {
                if (grabbed.isEmpty()) {
                    grabbed.set(stack);
                } else {
                    grabbed.clear();
                }
            } else {
                std::swap(grabbed, stack);
            }
        }
    } else if (button == mousecode::BUTTON_2) {
        if (layout.rightClick) {
            layout.rightClick(inventoryid, stack);
            if (layout.updateFunc) {
                layout.updateFunc(layout.index, stack);
            }
            return;
        }
        if (layout.itemSource)
            return;
        if (grabbed.isEmpty()) {
            if (!stack.isEmpty()) {
                grabbed.set(stack);
                int halfremain = stack.getCount() / 2;
                grabbed.setCount(stack.getCount() - halfremain);
                stack.setCount(halfremain);
            }
        } else {
            auto& stackDef =
                content->getIndices()->items.require(stack.getItemId());
            if (stack.isEmpty()) {
                stack.set(grabbed);
                stack.setCount(1);
                grabbed.setCount(grabbed.getCount() - 1);
            } else if (stack.accepts(grabbed) && stack.getCount() < stackDef.stackSize) {
                stack.setCount(stack.getCount() + 1);
                grabbed.setCount(grabbed.getCount() - 1);
            }
        }
    }
    if (layout.updateFunc) {
        layout.updateFunc(layout.index, stack);
    }
}

void SlotView::onFocus(gui::GUI* gui) {
    clicked(gui, mousecode::BUTTON_1);
}

const std::wstring& SlotView::getTooltip() const {
    const auto& str = UINode::getTooltip();
    if (!str.empty() || tooltip.empty()) {
        return str;
    }
    return tooltip;
}

void SlotView::bind(
    int64_t inventoryid,
    ItemStack& stack, 
    const Content* content
) {
    this->inventoryid = inventoryid;
    bound = &stack;
    this->content = content;
}

const SlotLayout& SlotView::getLayout() const {
    return layout;
}

ItemStack& SlotView::getStack() {
    return *bound;
}

InventoryView::InventoryView() : Container(glm::vec2()) {
    setColor(glm::vec4(0, 0, 0, 0.0f));
}

InventoryView::~InventoryView() {}


std::shared_ptr<SlotView> InventoryView::addSlot(const SlotLayout& layout) {
    uint width =  InventoryView::SLOT_SIZE + layout.padding;
    uint height = InventoryView::SLOT_SIZE + layout.padding;

    auto pos = layout.position;
    auto vsize = getSize();
    if (pos.x + width > vsize.x) {
        vsize.x = pos.x + width;
    }
    if (pos.y + height > vsize.y) {
        vsize.y = pos.y + height;
    }
    setSize(vsize);

    auto slot = std::make_shared<SlotView>(layout);
    if (!layout.background) {
        slot->setColor(glm::vec4());
    }
    slots.push_back(slot.get());
    return slot;
}

std::shared_ptr<Inventory> InventoryView::getInventory() const {
    return inventory;
}


size_t InventoryView::getSlotsCount() const {
    return slots.size();
}

void InventoryView::bind(
    const std::shared_ptr<Inventory>& inventory,
    const Content* content
) {
    this->inventory = inventory;
    this->content = content;
    for (auto slot : slots) {
        slot->bind(
            inventory->getId(),
            inventory->getSlot(slot->getLayout().index),
            content
        );
    }
}

void InventoryView::unbind() {
    inventory = nullptr;
}

void InventoryView::setSelected(int index) {
    for (size_t i = 0; i < slots.size(); i++) {
        slots[i]->setHighlighted(static_cast<int>(i) == index);
    }
}

void InventoryView::setPos(glm::vec2 pos) {
    Container::setPos(pos - origin);
}

void InventoryView::setOrigin(glm::vec2 origin) {
    this->origin = origin;
}

glm::vec2 InventoryView::getOrigin() const {
    return origin;
}
