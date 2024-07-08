#include "Content.hpp"

#include <memory>
#include <stdexcept>
#include <glm/glm.hpp>
#include <utility>

#include "../voxels/Block.hpp"
#include "../items/ItemDef.hpp"
#include "../objects/EntityDef.hpp"
#include "../objects/rigging.hpp"

#include "ContentPack.hpp"
#include "../logic/scripting/scripting.hpp"

ContentIndices::ContentIndices(
    ContentUnitIndices<Block> blocks,
    ContentUnitIndices<ItemDef> items,
    ContentUnitIndices<EntityDef> entities
) : blocks(std::move(blocks)),
    items(std::move(items)),
    entities(std::move(entities))
{}

Content::Content(
    std::unique_ptr<ContentIndices> indices, 
    std::unique_ptr<DrawGroups> drawGroups,
    ContentUnitDefs<Block> blocks,
    ContentUnitDefs<ItemDef> items,
    ContentUnitDefs<EntityDef> entities,
    UptrsMap<std::string, ContentPackRuntime> packs,
    UptrsMap<std::string, BlockMaterial> blockMaterials,
    UptrsMap<std::string, rigging::RigConfig> rigs
) : indices(std::move(indices)),
    packs(std::move(packs)),
    blockMaterials(std::move(blockMaterials)),
    rigs(std::move(rigs)),
    blocks(std::move(blocks)),
    items(std::move(items)),
    entities(std::move(entities)),
    drawGroups(std::move(drawGroups))
{}

Content::~Content() {
}

const rigging::RigConfig* Content::getRig(const std::string& id) const {
    auto found = rigs.find(id);
    if (found == rigs.end()) {
        return nullptr;
    }
    return found->second.get();
}

const BlockMaterial* Content::findBlockMaterial(const std::string& id) const {
    auto found = blockMaterials.find(id);
    if (found == blockMaterials.end()) {
        return nullptr;
    }
    return found->second.get();
}

const ContentPackRuntime* Content::getPackRuntime(const std::string& id) const {
    auto found = packs.find(id);
    if (found == packs.end()) {
        return nullptr;
    }
    return found->second.get();
}

const UptrsMap<std::string, BlockMaterial>& Content::getBlockMaterials() const {
    return blockMaterials;
}

const UptrsMap<std::string, ContentPackRuntime>& Content::getPacks() const {
    return packs;
}
