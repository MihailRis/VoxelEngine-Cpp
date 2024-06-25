#include "Content.hpp"

#include <memory>
#include <stdexcept>
#include <glm/glm.hpp>
#include <utility>

#include "../voxels/Block.hpp"
#include "../items/ItemDef.hpp"

#include "ContentPack.hpp"
#include "../logic/scripting/scripting.hpp"

ContentIndices::ContentIndices(
    std::vector<Block*> blocks, 
    std::vector<ItemDef*> items
) : blocks(std::move(blocks)),
    items(std::move(items))
{}

Content::Content(
    std::unique_ptr<ContentIndices> indices, 
    std::unique_ptr<DrawGroups> drawGroups,
    ContentUnitDefs<Block> blocks,
    ContentUnitDefs<ItemDef> items,
    std::unordered_map<std::string, std::unique_ptr<ContentPackRuntime>> packs,
    std::unordered_map<std::string, std::unique_ptr<BlockMaterial>> blockMaterials
) : indices(std::move(indices)),
    packs(std::move(packs)),
    blockMaterials(std::move(blockMaterials)),
    blocks(std::move(blocks)),
    items(std::move(items)),
    drawGroups(std::move(drawGroups)) 
{}

Content::~Content() {
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

const std::unordered_map<std::string, std::unique_ptr<BlockMaterial>>& Content::getBlockMaterials() const {
    return blockMaterials;
}

const std::unordered_map<std::string, std::unique_ptr<ContentPackRuntime>>& Content::getPacks() const {
    return packs;
}
