#include "ContentLUT.h"

#include <memory>

#include "Content.h"
#include "../constants.h"
#include "../files/files.h"
#include "../coders/json.h"
#include "../voxels/Block.h"
#include "../items/ItemDef.h"

#include "../data/dynamic.h"

ContentLUT::ContentLUT(const Content* content, size_t blocksCount, size_t itemsCount) {
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

    for (size_t i = 0; i < itemsCount; i++) {
        items.push_back(i);
    }
    for (size_t i = 0; i < indices->countItemDefs(); i++) {
        itemNames.push_back(indices->getItemDef(i)->name);
    }
    for (size_t i = indices->countItemDefs(); i < itemsCount; i++) {
        itemNames.push_back("");
    }
}

ContentLUT* ContentLUT::create(const fs::path& filename, 
                               const Content* content) {
    auto root = files::read_json(filename);
    auto blocklist = root->list("blocks");
    auto itemlist = root->list("items");

    auto* indices = content->getIndices();
    size_t blocks_c = blocklist 
                      ? std::max(blocklist->size(), indices->countBlockDefs()) 
                      : indices->countBlockDefs();
    size_t items_c = itemlist 
                     ? std::max(itemlist->size(), indices->countItemDefs()) 
                     : indices->countItemDefs();    

    auto lut = std::make_unique<ContentLUT>(content, blocks_c, items_c);

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

    if (itemlist) {
        for (size_t i = 0; i < itemlist->size(); i++) {
            std::string name = itemlist->str(i);
            ItemDef* def = content->findItem(name);
            if (def) {
                lut->setItem(i, name, def->rt.id);
            } else {
                lut->setItem(i, name, ITEM_VOID);   
            }
        }
    }

    if (lut->hasContentReorder() || lut->hasMissingContent()) {
        return lut.release();
    } else {
        return nullptr;
    }
}
