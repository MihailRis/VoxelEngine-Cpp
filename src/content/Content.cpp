#include "Content.hpp"

#include <memory>
#include <stdexcept>
#include <glm/glm.hpp>

#include "../voxels/Block.hpp"
#include "../items/ItemDef.hpp"

#include "ContentPack.hpp"
#include "../logic/scripting/scripting.hpp"

ContentIndices::ContentIndices(
    std::vector<Block*> blockDefs, 
    std::vector<ItemDef*> itemDefs
) : blockDefs(blockDefs), 
    itemDefs(itemDefs) 
{}

Content::Content(
    std::unique_ptr<ContentIndices> indices, 
    std::unique_ptr<DrawGroups> drawGroups,
    std::unordered_map<std::string, std::unique_ptr<Block>> blockDefs,
    std::unordered_map<std::string, std::unique_ptr<ItemDef>> itemDefs,
    std::unordered_map<std::string, std::unique_ptr<ContentPackRuntime>> packs,
    std::unordered_map<std::string, std::unique_ptr<BlockMaterial>> blockMaterials
) : blockDefs(std::move(blockDefs)),
    itemDefs(std::move(itemDefs)),
    indices(std::move(indices)),
    packs(std::move(packs)),
    blockMaterials(std::move(blockMaterials)),
    drawGroups(std::move(drawGroups)) 
{}

Content::~Content() {
}

Block* Content::findBlock(std::string id) const {
    auto found = blockDefs.find(id);
    if (found == blockDefs.end()) {
        return nullptr;
    }
    return found->second.get();
}

Block& Content::requireBlock(std::string id) const {
    auto found = blockDefs.find(id);
    if (found == blockDefs.end()) {
        throw std::runtime_error("missing block "+id);
    }
    return *found->second;
}

ItemDef* Content::findItem(std::string id) const {
    auto found = itemDefs.find(id);
    if (found == itemDefs.end()) {
        return nullptr;
    }
    return found->second.get();
}

ItemDef& Content::requireItem(std::string id) const {
    auto found = itemDefs.find(id);
    if (found == itemDefs.end()) {
        throw std::runtime_error("missing item "+id);
    }
    return *found->second;
}

const BlockMaterial* Content::findBlockMaterial(std::string id) const {
    auto found = blockMaterials.find(id);
    if (found == blockMaterials.end()) {
        return nullptr;
    }
    return found->second.get();
}

const ContentPackRuntime* Content::getPackRuntime(std::string id) const {
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
