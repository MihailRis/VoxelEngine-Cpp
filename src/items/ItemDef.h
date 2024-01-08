#ifndef CONTENT_ITEMS_ITEM_DEF_H_
#define CONTENT_ITEMS_ITEM_DEF_H_

#include <string>
#include <glm/glm.hpp>

#include "../graphics/UVRegion.h"
#include "../typedefs.h"

struct item_funcs_set {
	bool init: 1;
};

enum class item_icon_type {
    none, // invisible (core:empty) must not be rendered
    sprite, // textured quad: icon is `atlas_name:texture_name`
    block, // block preview: icon is string block id
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
        UVRegion iconRegion {0, 0, 1, 1};
    } rt;

    ItemDef(std::string name);
};

#endif //CONTENT_ITEMS_ITEM_DEF_H_
