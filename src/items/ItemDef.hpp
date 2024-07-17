#ifndef CONTENT_ITEMS_ITEM_DEF_HPP_
#define CONTENT_ITEMS_ITEM_DEF_HPP_

#include <string>
#include <glm/glm.hpp>

#include "../typedefs.hpp"

struct item_funcs_set {
    bool init: 1;
    bool on_use: 1;
    bool on_use_on_block: 1;
    bool on_block_break_by: 1;
};

enum class item_icon_type {
    none, // invisible (core:empty) must not be rendered
    sprite, // textured quad: icon is `atlas_name:texture_name`
    block, // block preview: icon is string block id
};

struct ItemDef {
    /// @brief Item string id (with prefix included)
    std::string const name;

    /// @brief Item name will shown in inventory
    std::string caption;

    itemcount_t stackSize = 64;
    bool generated = false;
    uint8_t emission[4] {0, 0, 0, 0};

    item_icon_type iconType = item_icon_type::sprite;
    std::string icon = "blocks:notfound";

    std::string placingBlock = "core:air";
    std::string scriptName = name.substr(name.find(':')+1);

    struct {
        itemid_t id;
        item_funcs_set funcsset {};
        blockid_t placingBlock;
        bool emissive = false;
    } rt {};

    ItemDef(const std::string& name);
    ItemDef(const ItemDef&) = delete;
};

#endif //CONTENT_ITEMS_ITEM_DEF_HPP_
