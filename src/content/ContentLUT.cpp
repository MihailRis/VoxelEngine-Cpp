#include "ContentLUT.h"

#include <memory>

#include "Content.h"
#include "../constants.h"
#include "../files/files.h"
#include "../coders/json.h"
#include "../voxels/Block.h"

using std::string;
using std::unique_ptr;
using std::make_unique;
using std::filesystem::path;

#include <iostream>

ContentLUT::ContentLUT(size_t blocksCount, const Content* content) {
    ContentIndices* indices = content->indices;
    for (size_t i = 0; i < blocksCount; i++) {
        blocks.push_back(i);
    }
    for (size_t i = 0; i < indices->countBlockDefs(); i++) {
        blockNames.push_back(indices->getBlockDef(i)->name);
    }

        for (size_t i = indices->countBlockDefs(); i < blocksCount; i++) {
        blockNames.push_back("");
    }
}

ContentLUT* ContentLUT::create(const path& filename, 
                               const Content* content) {
    unique_ptr<json::JObject> root(files::read_json(filename));
    json::JArray* blocksarr = root->arr("blocks");

    auto& indices = content->indices;
    size_t blocks_c = blocksarr 
                      ? std::max(blocksarr->size(), indices->countBlockDefs()) 
                      : indices->countBlockDefs();
    
    auto lut = make_unique<ContentLUT>(blocks_c, content);
    if (blocksarr) {
        for (size_t i = 0; i < blocksarr->size(); i++) {
            string name = blocksarr->str(i);
            Block* def = content->findBlock(name);
            if (def) {
                lut->setBlock(i, name, def->rt.id);
            } else {
                lut->setBlock(i, name, BLOCK_VOID);   
            }
        }
    }
    if (lut->hasContentReorder() || lut->hasMissingContent()) {
        return lut.release();
    } else {
        return nullptr;
    }
}
