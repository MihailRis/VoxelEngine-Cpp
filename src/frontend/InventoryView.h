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

class InventoryView {
    const Content* content;
    const ContentIndices* indices;
    std::vector<itemid_t> items;
    slotconsumer consumer = nullptr;
    LevelFrontend* frontend;

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
        std::vector<itemid_t> items);

    virtual ~InventoryView();

    virtual void actAndDraw(const GfxContext* ctx);

    void setPosition(int x, int y);
    int getWidth() const;
    int getHeight() const;
    void setSlotConsumer(slotconsumer consumer);
};

#endif // FRONTEND_INVENTORY_VIEW_H_
