#ifndef ITEMSTACK_H_
#define ITEMSTACK_H_

#include <string>

class Item;

class ItemStack {
public:
    Item* item;
    int count;

    ItemStack(Item* item, int count);
    explicit ItemStack(Item* item);
};


#endif //ITEMSTACK_H_
