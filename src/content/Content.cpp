#include "Content.h"

#include <memory>
#include <stdexcept>
#include <glm/glm.hpp>

#include "../voxels/Block.h"
#include "../items/ItemDef.h"

#include "ContentPack.h"
#include "../logic/scripting/scripting.h"

ContentBuilder::~ContentBuilder() {
}

void ContentBuilder::add(Block* def) {
    checkIdentifier(def->name);
    blockDefs[def->name] = def;
    blockIds.push_back(def->name);
}

void ContentBuilder::add(ItemDef* def) {
    checkIdentifier(def->name);
    itemDefs[def->name] = def;
    itemIds.push_back(def->name);
}

void ContentBuilder::add(ContentPackRuntime* pack) {
    packs.push_back(std::unique_ptr<ContentPackRuntime>(pack));
}

Block& ContentBuilder::createBlock(std::string id) {
    auto found = blockDefs.find(id);
    if (found != blockDefs.end()) {
        //return found->second;
        throw namereuse_error("name "+id+" is already used", contenttype::item);
    }
    Block* block = new Block(id);
    add(block);
    return *block;
}

ItemDef& ContentBuilder::createItem(std::string id) {
    auto found = itemDefs.find(id);
    if (found != itemDefs.end()) {
        if (found->second->generated) {
            return *found->second;
        }
        throw namereuse_error("name "+id+" is already used", contenttype::item);
    }
    ItemDef* item = new ItemDef(id);
    add(item);
    return *item;
}

void ContentBuilder::checkIdentifier(std::string id) {
    contenttype result;
    if (((result = checkContentType(id)) != contenttype::none)) {
        throw namereuse_error("name "+id+" is already used", result);
    }  
}

contenttype ContentBuilder::checkContentType(std::string id) {
    if (blockDefs.find(id) != blockDefs.end()) {
        return contenttype::block;
    }
    if (itemDefs.find(id) != itemDefs.end()) {
        return contenttype::item;
    }
    return contenttype::none;
}

Content* ContentBuilder::build() {
    std::vector<Block*> blockDefsIndices;
    auto groups = std::make_unique<DrawGroups>();
    for (const std::string& name : blockIds) {
        Block* def = blockDefs[name];
        
        // Generating runtime info
        def->rt.id = blockDefsIndices.size();
        def->rt.emissive = *reinterpret_cast<uint32_t*>(def->emission);
        def->rt.solid = def->model == BlockModel::block;

        if (def->rotatable) {
            for (uint i = 0; i < BlockRotProfile::MAX_COUNT; i++) {
                def->rt.hitboxes[i].reserve(def->hitboxes.size());
                for (AABB aabb : def->hitboxes) {
                    def->rotations.variants[i].transform(aabb);
                    def->rt.hitboxes[i].push_back(aabb);
                }
            }
        }

        blockDefsIndices.push_back(def);
        if (groups->find(def->drawGroup) == groups->end()) {
            groups->insert(def->drawGroup);
        }
    }

    std::vector<ItemDef*> itemDefsIndices;
    for (const std::string& name : itemIds) {
        ItemDef* def = itemDefs[name];
        
        // Generating runtime info
        def->rt.id = itemDefsIndices.size();
        def->rt.emissive = *reinterpret_cast<uint32_t*>(def->emission);
        itemDefsIndices.push_back(def);
    }

    auto indices = new ContentIndices(blockDefsIndices, itemDefsIndices);

    auto content = std::make_unique<Content>(
        indices, std::move(groups), blockDefs, itemDefs, std::move(packs)
    );

    // Now, it's time to resolve foreign keys
    for (Block* def : blockDefsIndices) {
        def->rt.pickingItem = content->requireItem(def->pickingItem).rt.id;
    }

    for (ItemDef* def : itemDefsIndices) {
        def->rt.placingBlock = content->requireBlock(def->placingBlock).rt.id;
    }

    return content.release();
}

ContentIndices::ContentIndices(
    std::vector<Block*> blockDefs, 
    std::vector<ItemDef*> itemDefs)
    : blockDefs(blockDefs), 
      itemDefs(itemDefs) {
}

Content::Content(ContentIndices* indices, 
                 std::unique_ptr<DrawGroups> drawGroups,
                 std::unordered_map<std::string, Block*> blockDefs,
                 std::unordered_map<std::string, ItemDef*> itemDefs,
                 std::vector<std::unique_ptr<ContentPackRuntime>> packs)
        : blockDefs(blockDefs),
          itemDefs(itemDefs),
          indices(indices),
          packs(std::move(packs)),
          drawGroups(std::move(drawGroups)) {
}

Content::~Content() {
}

Block* Content::findBlock(std::string id) const {
    auto found = blockDefs.find(id);
    if (found == blockDefs.end()) {
        return nullptr;
    }
    return found->second;
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
    return found->second;
}

ItemDef& Content::requireItem(std::string id) const {
    auto found = itemDefs.find(id);
    if (found == itemDefs.end()) {
        throw std::runtime_error("missing item "+id);
    }
    return *found->second;
}

const std::vector<std::unique_ptr<ContentPackRuntime>>& Content::getPacks() const {
    return packs;
}
