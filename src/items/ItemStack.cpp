#include "ItemStack.h"

#include "ItemDef.h"
#include "../content/Content.h"

ItemStack::ItemStack() : item(ITEM_EMPTY), count(0) {
}

ItemStack::ItemStack(itemid_t item, itemcount_t count) : item(item), count(count) {
}

void ItemStack::set(const ItemStack& item) {
    this->item = item.item;
    this->count = item.count;
}

bool ItemStack::accepts(const ItemStack& other) const {
    if (isEmpty()) {
        return true;
    }
    return item == other.getItemId();
}

void ItemStack::move(ItemStack& item, const ContentIndices* indices) {
    auto def = indices->getItemDef(item.getItemId());
    int count = std::min(item.count, def->stackSize-this->count);
    if (isEmpty()) {
        set(ItemStack(item.getItemId(), count));
    } else {
        setCount(this->count + count);
    }
    item.setCount(item.count-count);
}

void ItemStack::setCount(itemcount_t count) {
    this->count = count;
    if (count == 0) {
        item = 0;
    }
}
