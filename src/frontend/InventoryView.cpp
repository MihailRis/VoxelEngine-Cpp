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
#include "../logic/scripting/scripting.h"

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

InventoryBuilder::InventoryBuilder() {
    view = std::make_shared<InventoryView>();
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
            add(builtSlot);
        }
    }
}

void InventoryBuilder::add(SlotLayout layout) {
    view->add(view->addSlot(layout), layout.position);
}

std::shared_ptr<InventoryView> InventoryBuilder::build() {
    return view;
}

SlotView::SlotView(
    SlotLayout layout
) : UINode(glm::vec2(), glm::vec2(InventoryView::SLOT_SIZE)),
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
            const Block& cblock = content->requireBlock(item->icon);
            batch->texture(previews->getTexture());

            UVRegion region = previews->get(cblock.name);
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

    ItemStack& grabbed = interaction->getGrabbedItem();
    ItemStack& stack = *bound;
    
    if (button == mousecode::BUTTON_1) {
        if (Events::pressed(keycode::LEFT_SHIFT)) {
            if (layout.shareFunc) {
                layout.shareFunc(layout.index, stack);
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

void SlotView::bind(
    ItemStack& stack, 
    LevelFrontend* frontend, 
    InventoryInteraction* interaction
) {
    bound = &stack;
    content = frontend->getLevel()->content;
    this->frontend = frontend;
    this->interaction = interaction;
}

const SlotLayout& SlotView::getLayout() const {
    return layout;
}

InventoryView::InventoryView() : Container(glm::vec2(), glm::vec2()) {
    setColor(glm::vec4(0, 0, 0, 0.0f));
}

InventoryView::~InventoryView() {}


std::shared_ptr<SlotView> InventoryView::addSlot(SlotLayout layout) {
    uint width =  InventoryView::SLOT_SIZE + layout.padding;
    uint height = InventoryView::SLOT_SIZE + layout.padding;

    auto coord = layout.position;
    auto vsize = getSize();
    if (coord.x + width > vsize.x) {
        vsize.x = coord.x + width;
    }
    if (coord.y + height > vsize.y) {
        vsize.y = coord.y + height;
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

void InventoryView::bind(
    std::shared_ptr<Inventory> inventory,
    LevelFrontend* frontend, 
    InventoryInteraction* interaction
) {
    this->frontend = frontend;
    this->interaction = interaction;
    this->inventory = inventory;
    content = frontend->getLevel()->content;
    indices = content->getIndices();
    for (auto slot : slots) {
        slot->bind(
            inventory->getSlot(slot->getLayout().index),
            frontend, interaction
        );
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

#include "../coders/xml.h"
#include "gui/gui_xml.h"

static itemsharefunc readShareFunc(InventoryView* view, gui::UiXmlReader& reader, xml::xmlelement& element) {
    auto consumer = scripting::create_int_array_consumer(
        reader.getEnvironment().getId(), 
        element->attr("sharefunc").getText()
    );
    return [=](uint slot, ItemStack& stack) {
        int args[] {int(view->getInventory()->getId()), int(slot)};
        consumer(args, 2);
    };
}

static void readSlot(InventoryView* view, gui::UiXmlReader& reader, xml::xmlelement element) {
    int index = element->attr("index", "0").asInt();
    bool itemSource = element->attr("item-source", "false").asBool();
    SlotLayout layout(index, glm::vec2(), true, itemSource, nullptr, nullptr);
    if (element->has("coord")) {
        layout.position = element->attr("coord").asVec2();
    }
    if (element->has("sharefunc")) {
        layout.shareFunc = readShareFunc(view, reader, element);
    }
    auto slot = view->addSlot(layout);
    reader.readUINode(reader, element, *slot);
    view->add(slot);
}

static void readSlotsGrid(InventoryView* view, gui::UiXmlReader& reader, xml::xmlelement element) {
    int startIndex = element->attr("start-index", "0").asInt();
    int rows = element->attr("rows", "0").asInt();
    int cols = element->attr("cols", "0").asInt();
    int count = element->attr("count", "0").asInt();
    const int slotSize = InventoryView::SLOT_SIZE;
    int interval = element->attr("interval", "-1").asInt();
    if (interval < 0) {
        interval = InventoryView::SLOT_INTERVAL;
    }
    int padding = element->attr("padding", "-1").asInt();
    if (padding < 0) {
        padding = interval;
    }
    if (rows == 0) {
        rows = ceildiv(count, cols);
    } else if (cols == 0) {
        cols = ceildiv(count, rows);
    } else if (count == 0) {
        count = rows * cols;
    }
    bool itemSource = element->attr("item-source", "false").asBool();
    SlotLayout layout(-1, glm::vec2(), true, itemSource, nullptr, nullptr);
    if (element->has("coord")) {
        layout.position = element->attr("coord").asVec2();
    }
    if (element->has("sharefunc")) {
        layout.shareFunc = readShareFunc(view, reader, element);
    }
    layout.padding = padding;

    glm::vec2 size (
        cols * slotSize + (cols - 1) * interval + padding * 2,
        rows * slotSize + (rows - 1) * interval + padding * 2
    );
    int idx = 0;
    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < cols; col++, idx++) {
            if (idx >= count) {
                return;
            }
            SlotLayout slotLayout = layout;
            slotLayout.index = startIndex + idx;
            slotLayout.position += glm::vec2(
                padding + col * (slotSize + interval),
                padding + row * (slotSize + interval)
            );
            auto slot = view->addSlot(slotLayout);
            view->add(slot, slotLayout.position);
        }
    }
}

std::shared_ptr<InventoryView> InventoryView::readXML(
    const std::string& src,
    const std::string& file,
    const scripting::Environment& env
) {
    auto view = std::make_shared<InventoryView>();

    gui::UiXmlReader reader(env);
    createReaders(reader);

    auto document = xml::parse(file, src);
    auto root = document->getRoot();
    if (root->getTag() != "inventory") {
        throw std::runtime_error("'inventory' element expected");
    }
    return std::dynamic_pointer_cast<InventoryView>(reader.readXML(file, root));
}

void InventoryView::createReaders(gui::UiXmlReader& reader) {
    reader.add("inventory", [=](gui::UiXmlReader& reader, xml::xmlelement element) {
        auto view = std::make_shared<InventoryView>();
        reader.readUINode(reader, element, *view, true);

        for (auto& sub : element->getElements()) {
            if (sub->getTag() == "slot") {
                readSlot(view.get(), reader, sub);
            } else if (sub->getTag() == "slots-grid") {
                readSlotsGrid(view.get(), reader, sub);
            }
        }
        return view;
    });
}
