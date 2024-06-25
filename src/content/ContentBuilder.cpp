#include "ContentBuilder.hpp"

ContentBuilder::~ContentBuilder() {}

void ContentBuilder::add(std::unique_ptr<ContentPackRuntime> pack) {
    packs[pack->getId()] = std::move(pack);
}

Block& ContentBuilder::createBlock(const std::string& id) {
    auto found = blockDefs.find(id);
    if (found != blockDefs.end()) {
        return *found->second;
    }
    checkIdentifier(id);
    blockIds.push_back(id);
    blockDefs[id] = std::make_unique<Block>(id);
    return *blockDefs[id];
}

ItemDef& ContentBuilder::createItem(const std::string& id) {
    auto found = itemDefs.find(id);
    if (found != itemDefs.end()) {
        return *found->second;
    }
    checkIdentifier(id);
    itemIds.push_back(id);
    itemDefs[id] = std::make_unique<ItemDef>(id);
    return *itemDefs[id];
}

BlockMaterial& ContentBuilder::createBlockMaterial(const std::string& id) {
    blockMaterials[id] = std::make_unique<BlockMaterial>();
    auto& material = *blockMaterials[id];
    material.name = id;
    return material;
}

void ContentBuilder::checkIdentifier(const std::string& id) {
    contenttype result;
    if (((result = checkContentType(id)) != contenttype::none)) {
        throw namereuse_error("name "+id+" is already used", result);
    }  
}

contenttype ContentBuilder::checkContentType(const std::string& id) {
    if (blockDefs.find(id) != blockDefs.end()) {
        return contenttype::block;
    }
    if (itemDefs.find(id) != itemDefs.end()) {
        return contenttype::item;
    }
    return contenttype::none;
}

std::unique_ptr<Content> ContentBuilder::build() {
    std::vector<Block*> blockDefsIndices;
    auto groups = std::make_unique<DrawGroups>();
    for (const std::string& name : blockIds) {
        Block& def = *blockDefs[name];
        
        // Generating runtime info
        def.rt.id = blockDefsIndices.size();
        def.rt.emissive = *reinterpret_cast<uint32_t*>(def.emission);
        def.rt.solid = def.model == BlockModel::block;
        def.rt.extended = def.size.x > 1 || def.size.y > 1 || def.size.z > 1;

        if (def.rotatable) {
            for (uint i = 0; i < BlockRotProfile::MAX_COUNT; i++) {
                def.rt.hitboxes[i].reserve(def.hitboxes.size());
                for (AABB aabb : def.hitboxes) {
                    def.rotations.variants[i].transform(aabb);
                    def.rt.hitboxes[i].push_back(aabb);
                }
            }
        }

        blockDefsIndices.push_back(&def);
        groups->insert(def.drawGroup);
    }

    std::vector<ItemDef*> itemDefsIndices;
    for (const std::string& name : itemIds) {
        ItemDef& def = *itemDefs[name];
        
        // Generating runtime info
        def.rt.id = itemDefsIndices.size();
        def.rt.emissive = *reinterpret_cast<uint32_t*>(def.emission);
        itemDefsIndices.push_back(&def);
    }

    auto content = std::make_unique<Content>(
        std::make_unique<ContentIndices>(blockDefsIndices, itemDefsIndices), 
        std::move(groups), 
        std::move(blockDefs), 
        std::move(itemDefs), 
        std::move(packs), 
        std::move(blockMaterials)
    );

    // Now, it's time to resolve foreign keys
    for (Block* def : blockDefsIndices) {
        def->rt.pickingItem = content->items.require(def->pickingItem).rt.id;
    }

    for (ItemDef* def : itemDefsIndices) {
        def->rt.placingBlock = content->blocks.require(def->placingBlock).rt.id;
    }

    return content;
}
