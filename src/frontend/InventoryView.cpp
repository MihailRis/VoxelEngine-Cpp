#include "InventoryView.h"

#include <iostream>
#include <glm/glm.hpp>

#include "BlocksPreview.h"
#include "LevelFrontend.h"
#include "../window/Events.h"
#include "../window/input.h"
#include "../assets/Assets.h"
#include "../graphics/Atlas.h"
#include "../graphics/Shader.h"
#include "../graphics/Batch2D.h"
#include "../graphics/GfxContext.h"
#include "../graphics/Font.h"
#include "../content/Content.h"
#include "../items/ItemDef.h"
#include "../items/Inventory.h"
#include "../maths/voxmaths.h"
#include "../objects/Player.h"
#include "../voxels/Block.h"
#include "../frontend/gui/controls.h"
#include "../util/stringutil.h"

InventoryLayout::InventoryLayout(glm::vec2 size) : size(size) {}

void InventoryLayout::add(SlotLayout slot) {
    slots.push_back(slot);
}

void InventoryLayout::add(InventoryPanel panel) {
    panels.push_back(panel);
}

void InventoryLayout::setSize(glm::vec2 size) {
    this->size = size;
}

void InventoryLayout::setOrigin(glm::vec2 origin) {
    this->origin = origin;
}

glm::vec2 InventoryLayout::getSize() const {
    return size;
}

glm::vec2 InventoryLayout::getOrigin() const {
    return origin;
}

std::vector<SlotLayout>& InventoryLayout::getSlots() {
    return slots;
}

std::vector<InventoryPanel>& InventoryLayout::getPanels() {
    return panels;
}

SlotLayout::SlotLayout(
    glm::vec2 position,
    bool background,
    bool itemSource,
    itemsharefunc shareFunc,
    slotcallback rightClick
) 
    : position(position),
      background(background),
      itemSource(itemSource),
      shareFunc(shareFunc),
      rightClick(rightClick) {}

InventoryPanel::InventoryPanel(
    glm::vec2 position,
    glm::vec2 size,
    glm::vec4 color)
    : position(position), size(size), color(color) {}

InventoryBuilder::InventoryBuilder() 
    : layout(std::make_unique<InventoryLayout>(glm::vec2()))
{}

void InventoryBuilder::addGrid(
    int cols, int count, 
    glm::vec2 coord, 
    int padding,
    bool addpanel,
    SlotLayout slotLayout) 
{
    const int slotSize = InventoryView::SLOT_SIZE;
    const int interval = InventoryView::SLOT_INTERVAL;

    int rows = ceildiv(count, cols);

    uint width =  cols * (slotSize + interval) - interval + padding*2;
    uint height = rows * (slotSize + interval) - interval + padding*2;
    
    auto lsize = layout->getSize();
    if (coord.x + width > lsize.x) {
        lsize.x = coord.x + width;
    }
    if (coord.y + height > lsize.y) {
        lsize.y = coord.y + height;
    }
    layout->setSize(lsize);

    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < cols; col++) {
            if (row * cols + col >= count)
                break;
            glm::vec2 position (
                col * (slotSize + interval) + padding,
                row * (slotSize + interval) + padding
            );
            auto builtSlot = slotLayout;
            builtSlot.position = position;
            layout->add(builtSlot);
        }
    }

    if (addpanel) {
        add(InventoryPanel(
            coord, 
            glm::vec2(width, height), 
            glm::vec4(0, 0, 0, 0.5f)));
    }
}

void InventoryBuilder::add(SlotLayout slotLayout) {
    uint width =  InventoryView::SLOT_SIZE;
    uint height = InventoryView::SLOT_SIZE;

    auto coord = slotLayout.position;
    auto lsize = layout->getSize();
    if (coord.x + width > lsize.x) {
        lsize.x = coord.x + width;
    }
    if (coord.y + height > lsize.y) {
        lsize.y = coord.y + height;
    }
    layout->add(slotLayout);
}

void InventoryBuilder::add(InventoryPanel panel) {
    layout->add(panel);
}

std::unique_ptr<InventoryLayout> InventoryBuilder::build() {
    return std::unique_ptr<InventoryLayout>(layout.release());
}

SlotView::SlotView(
    ItemStack& stack, 
    LevelFrontend* frontend,
    InventoryInteraction* interaction,
    const Content* content,
    SlotLayout layout) 
    : UINode(glm::vec2(), glm::vec2(InventoryView::SLOT_SIZE)), 
      frontend(frontend),
      interaction(interaction),
      content(content),
      stack(stack),
      layout(layout) {
    color(glm::vec4(0, 0, 0, 0.2f));
}

// performance disaster
void SlotView::draw(Batch2D* batch, Assets* assets) {
    glm::vec2 coord = calcCoord();

    int slotSize = InventoryView::SLOT_SIZE;

    glm::vec4 tint(1.0f);
    glm::vec4 color = color_;
    if (hover_ || highlighted) {
        tint *= 1.333f;
        color = glm::vec4(1, 1, 1, 0.2f);
    }

    batch->color = color;
    if (color.a > 0.0) {
        batch->texture(nullptr);
        if (highlighted) {
            batch->rect(coord.x-4, coord.y-4, slotSize+8, slotSize+8);
        } else {
            batch->rect(coord.x, coord.y, slotSize, slotSize);
        }
    }
    
    batch->color = glm::vec4(1.0f);

    auto previews = frontend->getBlocksAtlas();
    auto indices = content->getIndices();

    ItemDef* item = indices->getItemDef(stack.getItemId());
    switch (item->iconType) {
        case item_icon_type::none:
            break;
        case item_icon_type::block: {
            Block* cblock = content->requireBlock(item->icon);
            batch->texture(previews->getTexture());

            UVRegion region = previews->get(cblock->name);
            batch->rect(
                coord.x, coord.y, slotSize, slotSize, 
                0, 0, 0, region, false, true, tint);
            break;
        }
        case item_icon_type::sprite: {
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
            batch->rect(
                coord.x, coord.y, slotSize, slotSize, 
                0, 0, 0, region, false, true, tint);
            break;
        }
    }

    if (stack.getCount() > 1) {
        auto font = assets->getFont("normal");
        std::wstring text = std::to_wstring(stack.getCount());

        int x = coord.x+slotSize-text.length()*8;
        int y = coord.y+slotSize-16;

        batch->color = glm::vec4(0, 0, 0, 1.0f);
        font->draw(batch, text, x+1, y+1);
        batch->color = glm::vec4(1.0f);
        font->draw(batch, text, x, y);
    }
}

void SlotView::setHighlighted(bool flag) {
    highlighted = flag;
}

bool SlotView::isHighlighted() const {
    return highlighted;
}

void SlotView::clicked(gui::GUI* gui, int button) {
    ItemStack& grabbed = interaction->getGrabbedItem();
    if (button == mousecode::BUTTON_1) {
        if (Events::pressed(keycode::LEFT_SHIFT)) {
            if (layout.shareFunc) {
                layout.shareFunc(stack);
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
            layout.rightClick(stack, grabbed);
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
            if (stack.isEmpty()) {
                stack.set(grabbed);
                stack.setCount(1);
            } else {
                stack.setCount(stack.getCount()+1);
            }
            grabbed.setCount(grabbed.getCount()-1);
        }
    }
}

InventoryView::InventoryView(
            const Content* content,
            LevelFrontend* frontend,
            InventoryInteraction* interaction,
            std::shared_ptr<Inventory> inventory,
            std::unique_ptr<InventoryLayout> layout) 
            : Container(glm::vec2(), glm::vec2()),
              content(content),
              indices(content->getIndices()), 
              inventory(inventory),
              layout(std::move(layout)),
              frontend(frontend),
              interaction(interaction) {
    size(this->layout->getSize());
    color(glm::vec4(0, 0, 0, 0.0f));
}

InventoryView::~InventoryView() {}

void InventoryView::build() {
    size_t index = 0;
    for (auto& slot : layout->getSlots()) {
        if (index >= inventory->size())
            break;

        ItemStack& item = inventory->getSlot(index);

        auto view = std::make_shared<SlotView>(
            item, frontend, interaction, content, slot
        );
        if (!slot.background) {
            view->color(glm::vec4());
        }
        slots.push_back(view.get());
        add(view, slot.position);
        index++;
    }
}

void InventoryView::setSelected(int index) {
    for (int i = 0; i < int(slots.size()); i++) {
        auto slot = slots[i];
        slot->setHighlighted(i == index);
    }
}

void InventoryView::setCoord(glm::vec2 coord) {
    Container::setCoord(coord - layout->getOrigin());
}

void InventoryView::setInventory(std::shared_ptr<Inventory> inventory) {
    this->inventory = inventory;
}

InventoryLayout* InventoryView::getLayout() const {
    return layout.get();
}

void InventoryView::drawBackground(Batch2D* batch, Assets* assets) {
    glm::vec2 coord = calcCoord();

    batch->texture(nullptr);

    for (auto& panel : layout->getPanels()) {
        glm::vec2 size = panel.size;
        glm::vec2 pos = coord + panel.position;
        batch->color = panel.color;
        batch->rect(pos.x-1, pos.y-1, size.x+2, size.y+2);
    }
}
