#ifndef CONTENT_CONTENT_LUT_H_
#define CONTENT_CONTENT_LUT_H_

#include <string>
#include <vector>
#include <filesystem>

#include "../typedefs.h"
#include "../constants.h"

namespace fs = std::filesystem;

class Content;

// TODO: make it unified for all types of content

/* Content indices lookup table or report 
   used to convert world with different indices
   Building with indices.json */
class ContentLUT {
    std::vector<blockid_t> blocks;
    std::vector<std::string> blockNames;

    std::vector<itemid_t> items;
    std::vector<std::string> itemNames;

    bool reorderContent = false;
    bool missingContent = false;
public:
    ContentLUT(const Content* content, size_t blocks, size_t items);

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

    inline const std::string& getItemName(blockid_t index) const {
        return itemNames[index];
    }

    inline itemid_t getItemId(itemid_t index) const {
        return items[index];
    }

    inline void setItem(itemid_t index, std::string name, itemid_t id) {
        items[index] = id;
        itemNames[index] = name;
        if (id == ITEM_VOID) {
            missingContent = true;
        } else if (index != id) {
            reorderContent = true;
        }
    }

    static ContentLUT* create(
        const fs::path& filename, 
        const Content* content
    );
    
    inline bool hasContentReorder() const {
        return reorderContent;
    }
    inline bool hasMissingContent() const {
        return missingContent;
    }

    inline size_t countBlocks() const {
        return blocks.size();
    }

    inline size_t countItems() const {
        return items.size();
    }
};

#endif // CONTENT_CONTENT_LUT_H_
