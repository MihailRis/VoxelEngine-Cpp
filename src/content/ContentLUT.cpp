#include "ContentLUT.h"

#include <memory>

#include "Content.h"
#include "../constants.h"
#include "../files/files.h"
#include "../coders/json.h"
#include "../voxels/Block.h"

#include "../data/dynamic.h"

ContentLUT::ContentLUT(size_t blocksCount, const Content* content) {
    auto* indices = content->getIndices();
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

ContentLUT* ContentLUT::create(const fs::path& filename, 
                               const Content* content) {
    auto root = files::read_json(filename);
    auto blocklist = root->list("blocks");

    auto* indices = content->getIndices();
    size_t blocks_c = blocklist 
                      ? std::max(blocklist->size(), indices->countBlockDefs()) 
                      : indices->countBlockDefs();
    
    auto lut = std::make_unique<ContentLUT>(blocks_c, content);
    if (blocklist) {
        for (size_t i = 0; i < blocklist->size(); i++) {
            std::string name = blocklist->str(i);
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
