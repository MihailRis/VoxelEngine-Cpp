#ifndef ITEM_H_
#define ITEM_H_

#include <string>
#include "../typedefs.h"

class Item {
public:
    std::string name;

    // TODO: item properties

    struct {
        itemid_t id;
    } rt;

    explicit Item(std::string name);
};


#endif //ITEM_H_
