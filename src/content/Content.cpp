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
        def->id = blockDefsIndices.size();
        def->rt.emissive = *((uint32_t*)def->emission);
        
        // build hitbox grid 3d for raycasts
        const AABB& hitbox = def->hitbox;
        for (uint gy = 0; gy < BLOCK_AABB_GRID; gy++) {
            for (uint gz = 0; gz < BLOCK_AABB_GRID; gz++) {
                for (uint gx = 0; gx < BLOCK_AABB_GRID; gx++) {
                    float x = gx / float(BLOCK_AABB_GRID);
                    float y = gy / float(BLOCK_AABB_GRID);
                    float z = gz / float(BLOCK_AABB_GRID);
                    bool flag = hitbox.inside({x, y, z});
                    if (!flag)
                        def->rt.solid = false;
                    def->rt.hitboxGrid[gy][gz][gx] = flag;
                }
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

ContentIndices::ContentIndices(vector<Block*> blockDefs)
               : blockDefs(blockDefs) {
}

Content::Content(ContentIndices* indices, DrawGroups* drawGroups,
                 unordered_map<string, Block*> blockDefs)
        : blockDefs(blockDefs),
          drawGroups(drawGroups),
          indices(indices) {
}

Content::~Content() {
    delete indices;
}

Block* Content::require(std::string id) const {
    auto found = blockDefs.find(id);
    if (found == blockDefs.end()) {
        throw std::runtime_error("missing block "+id);
    }
    return found->second;
}
