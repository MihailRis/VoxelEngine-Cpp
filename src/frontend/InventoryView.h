#ifndef FRONTEND_INVENTORY_VIEW_H_
#define FRONTEND_INVENTORY_VIEW_H_

#include <vector>
#include <functional>
#include <glm/glm.hpp>

#include "../frontend/gui/UINode.h"
#include "../frontend/gui/panels.h"
#include "../frontend/gui/controls.h"
#include "../items/ItemStack.h"
#include "../typedefs.h"

class Assets;
class GfxContext;
class Content;
class ContentIndices;
class LevelFrontend;
class Inventory;

using itemsharefunc = std::function<void(ItemStack&)>;
using slotcallback = std::function<void(ItemStack&, ItemStack&)>;

class InventoryInteraction {
    ItemStack grabbedItem;
public:
    InventoryInteraction() = default;

    ItemStack& getGrabbedItem() {
        return grabbedItem;
    }
};

struct SlotLayout {
    int index;
    glm::vec2 position;
    bool background;
    bool itemSource;
    itemsharefunc shareFunc;
    slotcallback rightClick;

    SlotLayout(int index,
               glm::vec2 position, 
               bool background,
               bool itemSource,
               itemsharefunc shareFunc,
               slotcallback rightClick);
};

class SlotView : public gui::UINode {
    LevelFrontend* frontend;
    InventoryInteraction& interaction;
    const Content* const content;
    SlotLayout layout;
    bool highlighted = false;

    ItemStack* bound = nullptr;
public:
    SlotView(LevelFrontend* frontend,
             InventoryInteraction& interaction,
             SlotLayout layout);

    virtual void draw(const GfxContext* pctx, Assets* assets) override;

    void setHighlighted(bool flag);
    bool isHighlighted() const;

    virtual void clicked(gui::GUI*, int) override;
    virtual void focus(gui::GUI*) override;

    void bind(ItemStack& stack);

    const SlotLayout& getLayout() const;
};

class InventoryView : public gui::Container {
    const Content* content;
    const ContentIndices* indices;
    
    std::shared_ptr<Inventory> inventory;
    LevelFrontend* frontend;
    InventoryInteraction& interaction;

    std::vector<SlotView*> slots;
    glm::vec2 origin {};
public:
    InventoryView(LevelFrontend* frontend, InventoryInteraction& interaction);
    virtual ~InventoryView();

    void setInventory(std::shared_ptr<Inventory> inventory);

    virtual void setCoord(glm::vec2 coord) override;

    void setOrigin(glm::vec2 origin);
    glm::vec2 getOrigin() const;

    void setSelected(int index);

    void bind(std::shared_ptr<Inventory> inventory);

    std::shared_ptr<SlotView> addSlot(SlotLayout layout);

    static std::shared_ptr<InventoryView> readXML(
        LevelFrontend* frontend, 
        InventoryInteraction& interaction,
        const std::string& src,
        const std::string& file
    );

    static const int SLOT_INTERVAL = 4;
    static const int SLOT_SIZE = ITEM_ICON_SIZE;
};

class InventoryBuilder {
    std::shared_ptr<InventoryView> view;
    LevelFrontend* frontend;
    InventoryInteraction& interaction;
public:
    InventoryBuilder(LevelFrontend* frontend, InventoryInteraction& interaction);

    void addGrid(
        int cols, int count, 
        glm::vec2 coord, 
        int padding,
        bool addpanel,
        SlotLayout slotLayout
    );
    
    void add(SlotLayout slotLayout);
    std::shared_ptr<InventoryView> build();
};

#endif // FRONTEND_INVENTORY_VIEW_H_
