#pragma once

#include <memory>
#include <vector>

#include "interfaces/Serializable.hpp"
#include "typedefs.hpp"
#include "ItemStack.hpp"

namespace dynamic {
    class Map;
}

class ContentLUT;
class ContentIndices;

class Inventory : public Serializable {
    int64_t id;
    std::vector<ItemStack> slots;
public:
    Inventory(int64_t id, size_t size);

    Inventory(const Inventory& orig);

    ItemStack& getSlot(size_t index);
    size_t findEmptySlot(size_t begin = 0, size_t end = -1) const;
    size_t findSlotByItem(itemid_t id, size_t begin = 0, size_t end = -1);

    inline size_t size() const {
        return slots.size();
    }

    void move(
        ItemStack& item,
        const ContentIndices* indices,
        size_t begin = 0,
        size_t end = -1
    );

    /* deserializing inventory */
    void deserialize(dynamic::Map* src) override;
    /* serializing inventory */
    std::unique_ptr<dynamic::Map> serialize() const override;

    static void convert(dynamic::Map* data, const ContentLUT* lut);

    inline void setId(int64_t id) {
        this->id = id;
    }

    inline int64_t getId() const {
        return id;
    }

    inline bool isVirtual() const {
        return id < 0;
    }

    static const size_t npos;
};
