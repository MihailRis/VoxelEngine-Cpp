#pragma once

#include <glm/glm.hpp>
#include <string>

#include "data/dv.hpp"
#include "typedefs.hpp"

struct ItemFuncsSet {
    bool init : 1;
    bool on_use : 1;
    bool on_use_on_block : 1;
    bool on_block_break_by : 1;
};

enum class ItemIconType {
    NONE,    // invisible (core:empty) must not be rendered
    SPRITE,  // textured quad: icon is `atlas_name:texture_name`
    BLOCK,   // block preview: icon is string block id
};

struct ItemDef {
    /// @brief Item string id (with prefix included)
    std::string const name;

    /// @brief Item name will shown in inventory
    std::string caption;

    dv::value properties = nullptr;

    itemcount_t stackSize = 64;
    bool generated = false;
    uint8_t emission[4] {0, 0, 0, 0};

    ItemIconType iconType = ItemIconType::SPRITE;
    std::string icon = "blocks:notfound";

    std::string placingBlock = "core:air";
    std::string scriptName = name.substr(name.find(':') + 1);

    std::string modelName = name + ".model";

    struct {
        itemid_t id;
        blockid_t placingBlock;
        ItemFuncsSet funcsset {};
        bool emissive = false;
    } rt {};

    ItemDef(const std::string& name);
    ItemDef(const ItemDef&) = delete;
    void cloneTo(ItemDef& dst);
};
