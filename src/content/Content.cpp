#include "Content.h"

#include <stdexcept>
#include <glm/glm.hpp>

#include "../voxels/Block.h"
#include "../content/ItemDef.h"

using glm::vec3;
using std::string;
using std::unordered_map;

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

void ContentBuilder::checkIdentifier(std::string id) {
    contenttype result;
    if ((checkContentType(id) != contenttype::none)) {
        throw contentindexreuse_error("identifier "+id+" is already used", result);
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
    DrawGroups* groups = new DrawGroups;
    for (const string& name : blockIds) {
        Block* def = blockDefs[name];
        
        // Generating runtime info
        def->rt.id = blockDefsIndices.size();
        def->rt.emissive = *((uint32_t*)def->emission);
        def->rt.solid = def->model == BlockModel::block;

        if (def->rotatable) {
            const AABB& hitbox = def->hitbox;
            for (uint i = 0; i < BlockRotProfile::MAX_COUNT; i++) {
                AABB aabb = hitbox;
                def->rotations.variants[i].transform(aabb);
                def->rt.hitboxes[i] = aabb;
            }
        }

        blockDefsIndices.push_back(def);
        if (groups->find(def->drawGroup) == groups->end()) {
            groups->insert(def->drawGroup);
        }
    }

    std::vector<ItemDef*> itemDefsIndices;
    for (const string& name : itemIds) {
        ItemDef* def = itemDefs[name];

        // Generating runtime info
        def->rt.id = itemDefsIndices.size();
        itemDefsIndices.push_back(def);
    }

    auto indices = new ContentIndices(blockDefsIndices, itemDefsIndices);
    return new Content(indices, groups, blockDefs);
}

ContentIndices::ContentIndices(
    std::vector<Block*> blockDefs, 
    std::vector<ItemDef*> itemDefs)
    : blockDefs(blockDefs), 
      itemDefs(itemDefs) {
}

Content::Content(ContentIndices* indices, DrawGroups* drawGroups,
                 unordered_map<string, Block*> blockDefs)
        : blockDefs(blockDefs),
          indices(indices),
          drawGroups(drawGroups) {
}

Content::~Content() {
    delete indices;
    delete drawGroups;
}

Block* Content::findBlock(string id) const {
    auto found = blockDefs.find(id);
    if (found == blockDefs.end()) {
        return nullptr;
    }
    return found->second;
}

Block* Content::requireBlock(string id) const {
    auto found = blockDefs.find(id);
    if (found == blockDefs.end()) {
        throw std::runtime_error("missing block "+id);
    }
    return found->second;
}

ItemDef* Content::findItem(string id) const {
    auto found = itemDefs.find(id);
    if (found == itemDefs.end()) {
        return nullptr;
    }
    return found->second;
}

ItemDef* Content::requireItem(string id) const {
    auto found = itemDefs.find(id);
    if (found == itemDefs.end()) {
        throw std::runtime_error("missing item "+id);
    }
    return found->second;
}
