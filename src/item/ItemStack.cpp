#include "ItemStack.h"

ItemStack::ItemStack(Item *item, int count) {
    this->item = item;
    this->count = count;
}

ItemStack::ItemStack(Item *item) : ItemStack(item, 1) {}
