#include "Content.h"

#include <stdexcept>

#include "../voxels/Block.h"

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
    for (const string& name : blockIds) {
        Block* def = blockDefs[name];
        def->id = blockDefsIndices.size();
        blockDefsIndices.push_back(def);
    }
    ContentIndices* indices = new ContentIndices(blockDefsIndices);
    return new Content(indices, blockDefs);
}

ContentIndices::ContentIndices(vector<Block*> blockDefs)
               : blockDefs(blockDefs) {
}

Content::Content(ContentIndices* indices,
                 unordered_map<string, Block*> blockDefs)
        : blockDefs(blockDefs),
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
