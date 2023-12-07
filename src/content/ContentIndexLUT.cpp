#include "ContentIndexLUT.h"

#include <iostream>
#include <memory>

#include "Content.h"
#include "../constants.h"
#include "../files/files.h"
#include "../coders/json.h"
#include "../voxels/Block.h"

using std::string;
using std::unique_ptr;
using std::filesystem::path;

ContentIndexLUT::ContentIndexLUT(size_t blocksCount, const Content* content) {
    blocks = new blockid_t[blocksCount];
    blockNames = new string[blocksCount];
    for (size_t i = 0; i < blocksCount; i++) {
        blocks[i] = i;
    }

    ContentIndices* indices = content->indices;
    for (size_t i = 0; i < indices->countBlockDefs(); i++) {
        blockNames[i] = indices->getBlockDef(i)->name;
    }
}

ContentIndexLUT::~ContentIndexLUT() {
    delete[] blockNames;
    delete[] blocks;
}

ContentIndexLUT* ContentIndexLUT::create(const path& filename, const Content* content) {
    auto& indices = content->indices;
    unique_ptr<json::JObject> root(files::read_json(filename));
    json::JArray* blocksarr = root->arr("blocks");

    size_t blocks_c = blocksarr 
                       ? std::max(blocksarr->size(), indices->countBlockDefs()) 
                       : indices->countBlockDefs();
    
    unique_ptr<ContentIndexLUT> lut(new ContentIndexLUT(blocks_c, content));

    bool conflicts = false;

    // TODO: implement world files convert feature using ContentIndexLUT report
    for (size_t i = 0; i < blocksarr->size(); i++) {
        string name = blocksarr->str(i);
        Block* def = content->findBlock(name);
        if (def) {
            if (i != def->rt.id) {
                std::cerr << "block id has changed from ";
                std::cerr << def->rt.id << " to " << i << std::endl;
                conflicts = true;
            }
            lut->setBlock(i, name, def->rt.id);
        } else {
            std::cerr << "unknown block: " << name << std::endl;
            lut->setBlock(i, name, i);   
            conflicts = true;
        }
    }
    if (conflicts) {
        return lut.release();
    } else {
        return nullptr;
    }
}
