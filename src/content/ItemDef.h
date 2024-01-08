#ifndef CONTENT_ITEM_DEF_H_
#define CONTENT_ITEM_DEF_H_

#include <string>
#include <glm/glm.hpp>

#include "../typedefs.h"

struct item_funcs_set {
	bool init: 1;
};

class ItemDef {
public:
    std::string name;

    struct {
        itemid_t id;
        item_funcs_set funcsset {};
    } rt;

    ItemDef(std::string name);
};

#endif //CONTENT_ITEM_DEF_H_
