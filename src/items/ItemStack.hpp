#ifndef ITEMS_ITEM_STACK_HPP_
#define ITEMS_ITEM_STACK_HPP_

#include "constants.hpp"
#include "typedefs.hpp"

class ContentIndices;

class ItemStack {
    itemid_t item;
    itemcount_t count;
public:
    ItemStack();

    ItemStack(itemid_t item, itemcount_t count);

    void set(const ItemStack& item);
    void setCount(itemcount_t count);

    bool accepts(const ItemStack& item) const;
    void move(ItemStack& item, const ContentIndices* indices);

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

#endif  // ITEMS_ITEM_STACK_HPP_
