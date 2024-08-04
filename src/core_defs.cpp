#include "core_defs.hpp"

#include "items/ItemDef.hpp"
#include <content/Content.hpp>
#include <content/ContentBuilder.hpp>
#include "files/files.hpp"
#include "files/engine_paths.hpp"
#include "window/Window.hpp"
#include "window/Events.hpp"
#include "window/input.hpp"
#include "voxels/Block.hpp"

// All in-game definitions (blocks, items, etc..)
void corecontent::setup(EnginePaths* paths, ContentBuilder* builder) {
    Block& block = builder->blocks.create("core:air");
    block.replaceable = true;
    block.drawGroup = 1;
    block.lightPassing = true;
    block.skyLightPassing = true;
    block.obstacle = false;
    block.selectable = false;
    block.model = BlockModel::none;
    block.pickingItem = "core:empty";

    ItemDef& item = builder->items.create("core:empty");
    item.iconType = item_icon_type::none;

    auto bindsFile = paths->getResources()/fs::path("bindings.toml");
    if (fs::is_regular_file(bindsFile)) {
        Events::loadBindings(
            bindsFile.u8string(), files::read_string(bindsFile)
        );
    }
}
