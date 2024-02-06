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
#include "../frontend/gui/panels.h"
#include "../frontend/gui/controls.h"
#include "../util/stringutil.h"
#include "../world/Level.h"

SlotLayout::SlotLayout(
    int index,
    glm::vec2 position,
    bool background,
    bool itemSource,
    itemsharefunc shareFunc,
    slotcallback rightClick
) 
    : index(index),
      position(position),
      background(background),
      itemSource(itemSource),
      shareFunc(shareFunc),
      rightClick(rightClick) {}

InventoryBuilder::InventoryBuilder(
    LevelFrontend* frontend, 
    InventoryInteraction& interaction
) : frontend(frontend), 
    interaction(interaction) 
{
    view = std::make_shared<InventoryView>(frontend, interaction);
}

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
    
    glm::vec2 vsize = view->getSize();
    if (coord.x + width > vsize.x) {
        vsize.x = coord.x + width;
    }
    if (coord.y + height > vsize.y) {
        vsize.y = coord.y + height;
    }
    view->setSize(vsize);

    if (addpanel) {
        auto panel = std::make_shared<gui::Container>(coord, glm::vec2(width, height));
        view->setColor(glm::vec4(0, 0, 0, 0.5f));
        view->add(panel);
    }

    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < cols; col++) {
            if (row * cols + col >= count)
                break;
            glm::vec2 position (
                col * (slotSize + interval) + padding,
                row * (slotSize + interval) + padding
            );
            auto builtSlot = slotLayout;
            builtSlot.index = row * cols + col;
            builtSlot.position = position;
            view->addSlot(builtSlot);
        }
    }
}

void InventoryBuilder::add(SlotLayout layout) {
    view->addSlot(layout);
}

std::shared_ptr<InventoryView> InventoryBuilder::build() {
    return view;
}

SlotView::SlotView(
    LevelFrontend* frontend,
    InventoryInteraction& interaction,
    SlotLayout layout
) : UINode(glm::vec2(), glm::vec2(InventoryView::SLOT_SIZE)), 
    frontend(frontend),
    interaction(interaction),
    content(frontend->getLevel()->content),
    layout(layout)
{
    setColor(glm::vec4(0, 0, 0, 0.2f));
}

void SlotView::draw(const GfxContext* pctx, Assets* assets) {
    if (bound == nullptr)
        throw std::runtime_error("unbound slot");
    ItemStack& stack = *bound;

    glm::vec2 coord = calcCoord();

    int slotSize = InventoryView::SLOT_SIZE;

    glm::vec4 tint(1.0f);
    glm::vec4 color = getColor();
    if (hover || highlighted) {
        tint *= 1.333f;
        color = glm::vec4(1, 1, 1, 0.2f);
    }

    auto batch = pctx->getBatch2D();
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
    if (bound == nullptr)
        throw std::runtime_error("unbound slot");

    ItemStack& grabbed = interaction.getGrabbedItem();
    ItemStack& stack = *bound;
    
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
                grabbed.setCount(grabbed.getCount()-1);
            } else if (stack.accepts(grabbed)){
                stack.setCount(stack.getCount()+1);
                grabbed.setCount(grabbed.getCount()-1);
            }
        }
    }
}

void SlotView::focus(gui::GUI* gui) {
    clicked(gui, 0);
}

void SlotView::bind(ItemStack& stack) {
    bound = &stack;
}

const SlotLayout& SlotView::getLayout() const {
    return layout;
}

InventoryView::InventoryView(
            LevelFrontend* frontend,
            InventoryInteraction& interaction
) : Container(glm::vec2(), glm::vec2()),
    frontend(frontend),
    interaction(interaction) 
{
    content = frontend->getLevel()->content;
    indices = content->getIndices();
    setColor(glm::vec4(0, 0, 0, 0.0f));
}

InventoryView::~InventoryView() {}


void InventoryView::addSlot(SlotLayout layout) {
    uint width =  InventoryView::SLOT_SIZE;
    uint height = InventoryView::SLOT_SIZE;

    auto coord = layout.position;
    auto vsize = getSize();
    if (coord.x + width > vsize.x) {
        vsize.x = coord.x + width;
    }
    if (coord.y + height > vsize.y) {
        vsize.y = coord.y + height;
    }

    auto slot = std::make_shared<SlotView>(
        frontend, interaction, layout
    );
    if (!layout.background) {
        slot->setColor(glm::vec4());
    }
    add(slot, layout.position);
    slots.push_back(slot.get());
}

void InventoryView::bind(std::shared_ptr<Inventory> inventory) {
    this->inventory = inventory;
    for (auto slot : slots) {
        slot->bind(inventory->getSlot(slot->getLayout().index));
    }
}

void InventoryView::setSelected(int index) {
    for (int i = 0; i < int(slots.size()); i++) {
        auto slot = slots[i];
        slot->setHighlighted(i == index);
    }
}

void InventoryView::setCoord(glm::vec2 coord) {
    Container::setCoord(coord - origin);
}

void InventoryView::setOrigin(glm::vec2 origin) {
    this->origin = origin;
}

glm::vec2 InventoryView::getOrigin() const {
    return origin;
}

void InventoryView::setInventory(std::shared_ptr<Inventory> inventory) {
    this->inventory = inventory;
}
