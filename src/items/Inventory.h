#ifndef ITEMS_INVENTORY_H_
#define ITEMS_INVENTORY_H_

#include <vector>
#include <memory>

#include "ItemStack.h"

#include "../typedefs.h"
#include "../data/dynamic.h"

class ContentLUT;
class ContentIndices;

// TODO: items indices fix
class Inventory {
    std::vector<ItemStack> slots;
public:
    Inventory(size_t size);

    ItemStack& getSlot(size_t index);
    size_t findEmptySlot(size_t begin=0, size_t end=-1) const;
    size_t findSlotByItem(itemid_t id, size_t begin=0, size_t end=-1);
    
    inline size_t size() const {
        return slots.size();
    }

    void move(
        ItemStack& item, 
        const ContentIndices* indices, 
        size_t begin=0, 
        size_t end=-1);

    /* deserializing inventory */
    void read(const dynamic::Map* src);
    /* serializing inventory */
    std::unique_ptr<dynamic::Map> write() const;

    static void convert(dynamic::Map* data, const ContentLUT* lut);

    static const size_t npos;
};

#endif // ITEMS_INVENTORY_H_
