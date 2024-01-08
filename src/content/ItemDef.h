#ifndef CONTENT_ITEM_DEF_H_
#define CONTENT_ITEM_DEF_H_

#include <string>
#include <glm/glm.hpp>

#include "../typedefs.h"

#define BLOCK_ITEM_SUFFIX ".item"

struct item_funcs_set {
	bool init: 1;
};

enum class item_icon_type {
    sprite, block,
};

class ItemDef {
public:
    std::string const name;

    bool generated = false;

    item_icon_type iconType = item_icon_type::sprite;
    std::string icon = "block:notfound";

    std::string placingBlock = "none";

    struct {
        itemid_t id;
        item_funcs_set funcsset {};
    } rt;

    ItemDef(std::string name);
};

#endif //CONTENT_ITEM_DEF_H_
