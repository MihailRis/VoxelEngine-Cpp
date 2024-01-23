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

class Batch2D;
class Assets;
class GfxContext;
class Content;
class ContentIndices;
class LevelFrontend;
class Inventory;

typedef std::function<void(ItemStack&)> itemsharefunc;
typedef std::function<void(ItemStack&, ItemStack&)> slotcallback;

class InventoryInteraction;

struct SlotLayout {
    glm::vec2 position;
    bool background;
    bool itemSource;
    itemsharefunc shareFunc;
    slotcallback rightClick;

    SlotLayout(glm::vec2 position, 
               bool background,
               bool itemSource,
               itemsharefunc shareFunc,
               slotcallback rightClick);
};

// temporary unused
struct InventoryPanel {
    glm::vec2 position;
    glm::vec2 size;
    glm::vec4 color;

    InventoryPanel(glm::vec2 position,
                   glm::vec2 size,
                   glm::vec4 color);
};

class InventoryLayout {
    glm::vec2 size {};
    glm::vec2 origin {};
    std::vector<SlotLayout> slots;
    std::vector<InventoryPanel> panels;
public:
    InventoryLayout(glm::vec2 size);

    void add(SlotLayout slot);
    void add(InventoryPanel panel);
    void setSize(glm::vec2 size);
    void setOrigin(glm::vec2 origin);

    glm::vec2 getSize() const;
    glm::vec2 getOrigin() const;

    std::vector<SlotLayout>& getSlots();
    std::vector<InventoryPanel>& getPanels();
};

class InventoryBuilder {
    std::unique_ptr<InventoryLayout> layout;
public:
    InventoryBuilder();

    void addGrid(
        int cols, int count, 
        glm::vec2 coord, 
        int padding,
        bool addpanel,
        SlotLayout slotLayout);
    
    void add(SlotLayout slotLayout);
    void add(InventoryPanel panel);

    std::unique_ptr<InventoryLayout> build();
};

class SlotView : public gui::UINode {
    LevelFrontend* frontend;
    InventoryInteraction* interaction;
    const Content* const content;
    ItemStack& stack;
    bool highlighted = false;

    SlotLayout layout;
public:
    SlotView(ItemStack& stack, 
             LevelFrontend* frontend,
             InventoryInteraction* interaction, 
             const Content* content,
             SlotLayout layout);

    virtual void draw(Batch2D* batch, Assets* assets) override;

    void setHighlighted(bool flag);
    bool isHighlighted() const;

    virtual void clicked(gui::GUI*, int) override;
};

class InventoryView : public gui::Container {
    const Content* content;
    const ContentIndices* indices;
    
    std::shared_ptr<Inventory> inventory;
    std::unique_ptr<InventoryLayout> layout;
    LevelFrontend* frontend;
    InventoryInteraction* interaction;

    std::vector<SlotView*> slots;

    int scroll = 0;
public:
    InventoryView(
        const Content* content, 
        LevelFrontend* frontend,
        InventoryInteraction* interaction,
        std::shared_ptr<Inventory> inventory,
        std::unique_ptr<InventoryLayout> layout);

    virtual ~InventoryView();

    void build();

    virtual void drawBackground(Batch2D* batch, Assets* assets) override;

    void setInventory(std::shared_ptr<Inventory> inventory);

    virtual void setCoord(glm::vec2 coord) override;

    InventoryLayout* getLayout() const;

    void setSelected(int index);

    static const int SLOT_INTERVAL = 4;
    static const int SLOT_SIZE = ITEM_ICON_SIZE;
};

class InventoryInteraction {
    ItemStack grabbedItem;
public:
    InventoryInteraction() = default;

    ItemStack& getGrabbedItem() {
        return grabbedItem;
    }
};

#endif // FRONTEND_INVENTORY_VIEW_H_
