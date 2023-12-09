#include "Content.h"

#include <stdexcept>
#include <glm/glm.hpp>

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

Content* ContentBuilder::build() {
    vector<Block*> blockDefsIndices;
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
    ContentIndices* indices = new ContentIndices(blockDefsIndices);
    return new Content(indices, groups, blockDefs);
}

ContentIndices::ContentIndices(const vector<Block*>& blockDefs)
               : blockDefs(blockDefs) {
}

Content::Content(ContentIndices* indices, DrawGroups* drawGroups,
                 const unordered_map<string, Block*>& blockDefs)
        : blockDefs(blockDefs),
          indices(indices),
          drawGroups(drawGroups) {
}

Content::~Content() {
    delete indices;
    delete drawGroups;
}

Block* Content::findBlock(const string& id) const {
    auto found = blockDefs.find(id);
    if (found == blockDefs.end()) {
        return nullptr;
    }
    return found->second;
}

Block* Content::requireBlock(const string& id) const {
    auto found = blockDefs.find(id);
    if (found == blockDefs.end()) {
        throw std::runtime_error("missing block "+id);
    }
    return found->second;
}
