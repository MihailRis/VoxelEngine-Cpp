#ifndef CONTENT_CONTENT_LUT_HPP_
#define CONTENT_CONTENT_LUT_HPP_

#include "Content.hpp"

#include "../typedefs.hpp"
#include "../constants.hpp"

#include <string>
#include <utility>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

struct contententry {
    contenttype type;
    std::string name;
};

// TODO: make it unified for all types of content

/// @brief Content indices lookup table or report 
/// used to convert world with different indices
/// Building with indices.json
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
        blockNames[index] = std::move(name);
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
        itemNames[index] = std::move(name);
        if (id == ITEM_VOID) {
            missingContent = true;
        } else if (index != id) {
            reorderContent = true;
        }
    }

    static std::shared_ptr<ContentLUT> create(
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

    std::vector<contententry> getMissingContent() const;
};

#endif // CONTENT_CONTENT_LUT_HPP_
