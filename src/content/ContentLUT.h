#ifndef CONTENT_CONTENT_LUT_H_
#define CONTENT_CONTENT_LUT_H_

#include <string>
#include <vector>
#include <filesystem>

#include "../typedefs.h"
#include "../constants.h"

class Content;

/* Content indices lookup table or report 
   used to convert world with different indices
   Building with indices.json */
class ContentLUT {
    std::vector<blockid_t> blocks;
    std::vector<std::string> blockNames;

    bool reorderContent = false;
    bool missingContent = false;
public:
    ContentLUT(size_t blocks, const Content* content);

    inline const std::string& getBlockName(blockid_t index) const {
        return blockNames[index];
    }

    inline blockid_t getBlockId(blockid_t index) const {
        return blocks[index];
    }

    inline void setBlock(blockid_t index, std::string name, blockid_t id) {
        blocks[index] = id;
        blockNames[index] = name;
        if (id == BLOCK_VOID) {
            missingContent = true;
        } else if (index != id) {
            reorderContent = true;
        }
    }

    static ContentLUT* create(const std::filesystem::path& filename, 
                                   const Content* content);
    
    inline bool hasContentReorder() const {
        return reorderContent;
    }
    inline bool hasMissingContent() const {
        return missingContent;
    }

    inline size_t countBlocks() const {
        return blocks.size();
    }
};

#endif // CONTENT_CONTENT_LUT_H_
