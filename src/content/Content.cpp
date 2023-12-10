#include "Content.h"

#include <stdexcept>
#include <glm/glm.hpp>

#include "../item/Item.h"
#include "../voxels/Block.h"

using glm::vec3;
using std::vector;
using std::string;
using std::unordered_map;

void ContentBuilder::add(Block* def) {
    if (blockDefs.find(def->name) != blockDefs.end()) {
        throw std::runtime_error("block name duplicate: "+def->name);
    }
    blockDefs[def->name] = def;
    blockIds.push_back(def->name);
}

void ContentBuilder::add(Item* def) {
    if (itemDefs.find(def->name) != itemDefs.end()) {
        throw std::runtime_error("item name duplicate: "+def->name);
    }
    itemDefs[def->name] = def;
    itemIds.push_back(def->name);
}

Content* ContentBuilder::build() {
    vector<Block*> blockDefsIndices;
    vector<Item*> itemDefsIndices;
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
    for (const string& name : itemIds) {
        Item* def = itemDefs[name];

        // Generating runtime info
        def->rt.id = itemDefsIndices.size();

        itemDefsIndices.push_back(def);
    }
    // TODO: item ids
    ContentIndices* indices = new ContentIndices(blockDefsIndices, itemDefsIndices);
    return new Content(indices, groups, blockDefs, itemDefs);
}

ContentIndices::ContentIndices(vector<Block*> blockDefs, vector<Item*> itemDefs)
               : blockDefs(blockDefs), itemDefs(itemDefs) {
}

Content::Content(ContentIndices* indices, DrawGroups* drawGroups,
                 unordered_map<string, Block*> blockDefs,
                 unordered_map<string, Item*> itemDefs)
        : blockDefs(blockDefs),
          itemDefs(itemDefs),
          indices(indices),
          drawGroups(drawGroups) {
}

Content::~Content() {
    delete indices;
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

Item* Content::findItem(string id) const {
    auto found = itemDefs.find(id);
    if (found == itemDefs.end()) {
        return nullptr;
    }
    return found->second;
}

Item* Content::requireItem(string id) const {
    auto found = itemDefs.find(id);
    if (found == itemDefs.end()) {
        throw std::runtime_error("missing item "+id);
    }
    return found->second;
}
