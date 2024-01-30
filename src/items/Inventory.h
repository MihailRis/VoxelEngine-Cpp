#ifndef ITEMS_INVENTORY_H_
#define ITEMS_INVENTORY_H_

#include <vector>
#include <memory>

#include "ItemStack.h"

#include "../typedefs.h"
#include "../data/dynamic.h"
#include "../interfaces/Serializable.h"

class ContentLUT;
class ContentIndices;

class Inventory : Serializable {
    uint id;
    std::vector<ItemStack> slots;
public:
    Inventory(uint id, size_t size);

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
    void deserialize(dynamic::Map* src) override;
    /* serializing inventory */
    std::unique_ptr<dynamic::Map> serialize() const override;

    static void convert(dynamic::Map* data, const ContentLUT* lut);

    inline uint getId() const {
        return id;
    }

    static const size_t npos;
};

#endif // ITEMS_INVENTORY_H_
