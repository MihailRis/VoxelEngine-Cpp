#ifndef FRONTEND_INVENTORY_VIEW_H_
#define FRONTEND_INVENTORY_VIEW_H_

#include <vector>
#include <functional>
#include <glm/glm.hpp>

#include "../graphics/ui/elements/UINode.h"
#include "../graphics/ui/elements/layout/Container.hpp"
#include "../items/ItemStack.h"
#include "../typedefs.h"

class Assets;
class GfxContext;
class Content;
class ContentIndices;
class LevelFrontend;
class Inventory;

namespace gui {
    class UiXmlReader;
}

namespace scripting {
    class Environment;
}

using slotcallback = std::function<void(uint, ItemStack&)>;

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
    slotcallback updateFunc;
    slotcallback shareFunc;
    slotcallback rightClick;
    int padding = 0;

    SlotLayout(
        int index,
        glm::vec2 position, 
        bool background,
        bool itemSource,
        slotcallback updateFunc,
        slotcallback shareFunc,
        slotcallback rightClick
    );
};

class SlotView : public gui::UINode {
    LevelFrontend* frontend = nullptr;
    InventoryInteraction* interaction = nullptr;
    const Content* content;
    SlotLayout layout;
    bool highlighted = false;

    int64_t inventoryid = 0;
    ItemStack* bound = nullptr;
public:
    SlotView(SlotLayout layout);

    virtual void draw(const GfxContext* pctx, Assets* assets) override;

    void setHighlighted(bool flag);
    bool isHighlighted() const;

    virtual void clicked(gui::GUI*, mousecode) override;
    virtual void onFocus(gui::GUI*) override;

    void bind(
        int64_t inventoryid,
        ItemStack& stack,
        LevelFrontend* frontend, 
        InventoryInteraction* interaction
    );

    const SlotLayout& getLayout() const;
};

class InventoryView : public gui::Container {
    const Content* content;
    const ContentIndices* indices;
    
    std::shared_ptr<Inventory> inventory;
    LevelFrontend* frontend = nullptr;
    InventoryInteraction* interaction = nullptr;

    std::vector<SlotView*> slots;
    glm::vec2 origin {};
public:
    InventoryView();
    virtual ~InventoryView();

    void setInventory(std::shared_ptr<Inventory> inventory);

    virtual void setPos(glm::vec2 pos) override;

    void setOrigin(glm::vec2 origin);
    glm::vec2 getOrigin() const;

    void setSelected(int index);

    void bind(
        std::shared_ptr<Inventory> inventory,
        LevelFrontend* frontend, 
        InventoryInteraction* interaction
    );
    
    void unbind();

    std::shared_ptr<SlotView> addSlot(SlotLayout layout);

    std::shared_ptr<Inventory> getInventory() const;

    size_t getSlotsCount() const;

    static void createReaders(gui::UiXmlReader& reader);

    static const int SLOT_INTERVAL = 4;
    static const int SLOT_SIZE = ITEM_ICON_SIZE;
};

class InventoryBuilder {
    std::shared_ptr<InventoryView> view;
public:
    InventoryBuilder();

    /// @brief Add slots grid to inventory view 
    /// @param cols grid columns
    /// @param count total number of grid slots
    /// @param pos position of the first slot of the grid
    /// @param padding additional space around the grid
    /// @param addpanel automatically create panel behind the grid
    /// with size including padding
    /// @param slotLayout slot settings (index and position are ignored)
    void addGrid(
        int cols, int count, 
        glm::vec2 pos, 
        int padding,
        bool addpanel,
        SlotLayout slotLayout
    );
    
    void add(SlotLayout slotLayout);
    std::shared_ptr<InventoryView> build();
};

#endif // FRONTEND_INVENTORY_VIEW_H_
