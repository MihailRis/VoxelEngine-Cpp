#pragma once

#include "constants.hpp"
#include "typedefs.hpp"

class ContentIndices;

class ItemStack {
    itemid_t item = ITEM_EMPTY;
    itemcount_t count = 0;
public:
    ItemStack() = default;

    ItemStack(itemid_t item, itemcount_t count);

    void set(const ItemStack& item);
    void setCount(itemcount_t count);

    bool accepts(const ItemStack& item) const;

    /// @brief Move items from one stack to another. 
    /// If the target stack is completely filled, the source stack will be reduced.
    /// @param item source stack
    /// @param indices content indices
    void move(ItemStack& item, const ContentIndices& indices);

    inline void clear() {
        set(ItemStack(0, 0));
    }

    inline bool isEmpty() const {
        return item == ITEM_EMPTY;
    }

    inline itemid_t getItemId() const {
        return item;
    }

    inline itemcount_t getCount() const {
        return count;
    }
};
