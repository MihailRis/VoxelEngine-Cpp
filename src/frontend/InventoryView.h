#ifndef FRONTEND_INVENTORY_VIEW_H_
#define FRONTEND_INVENTORY_VIEW_H_

#include <vector>
#include <functional>
#include <glm/glm.hpp>

#include "../typedefs.h"

class Assets;
class GfxContext;
class Content;
class ContentIndices;
class LevelFrontend;

typedef std::function<void(itemid_t)> slotconsumer;

struct ItemSlot {
    ItemSlot(itemid_t id = 0u, ubyte amount = 0u) : id(id), amount(amount) {};
    itemid_t id = 0u;
    ubyte amount = 0u;
};

class ItemStorage {
public:
    ItemStorage(size_t slots);
    ItemStorage(const std::vector<itemid_t>& items);
    ~ItemStorage();

    void setItems(const std::vector<itemid_t>& items);
    void setItem(size_t slot, itemid_t item);
    itemid_t viewItem(size_t slot) const;
    void clearSlot(size_t slot);
    size_t getSize();

protected:
    //  "itemid_t" may be replaced by the "ItemSlot" struct in the future.
    std::vector<itemid_t> items;
};

class InventoryView : public ItemStorage {
    const Content* content;
    const ContentIndices* indices;
    slotconsumer consumer = nullptr;
    LevelFrontend* frontend;

    int hoverSlot = -1;
    int scroll = 0;
    int columns;
    uint iconSize = 48;
    uint interval = 4;
    glm::ivec2 padding {interval, interval};
    glm::ivec2 position {0, 0};
public:
    InventoryView(
        int columns,
        const Content* content, 
        LevelFrontend* frontend,
        const std::vector<itemid_t>& items);

    virtual ~InventoryView();

    virtual void actAndDraw(const GfxContext* ctx);

    void setPosition(int x, int y);
    int getWidth() const;
    int getHeight() const;
    void setSlotConsumer(slotconsumer consumer);
    void setHoverSlot(int slot);
};

#endif // FRONTEND_INVENTORY_VIEW_H_
