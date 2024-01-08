#ifndef CONTENT_CONTENT_H_
#define CONTENT_CONTENT_H_

#include <string>
#include <vector>
#include <stdexcept>
#include <unordered_map>
#include <set>
#include "../typedefs.h"

typedef std::set<unsigned char> DrawGroups;

class Block;
class ItemDef;
class Content;

enum class contenttype {
    none, block, item
};

class contentindexreuse_error: public std::runtime_error {
    contenttype type;
public:
    contentindexreuse_error(const std::string& msg, contenttype type)
        : std::runtime_error(msg), type(type) {}

    inline contenttype getType() const {
        return type;
    }
};

class ContentBuilder {
    std::unordered_map<std::string, Block*> blockDefs;
    std::vector<std::string> blockIds;

    std::unordered_map<std::string, ItemDef*> itemDefs;
    std::vector<std::string> itemIds;
public:
    void add(Block* def);
    void add(ItemDef* def);

    void checkIdentifier(std::string id);
    contenttype checkContentType(std::string id);

    Content* build();
};

/* Runtime defs cache: indices */
class ContentIndices {
    std::vector<Block*> blockDefs;
    std::vector<ItemDef*> itemDefs;
public:
    ContentIndices(std::vector<Block*> blockDefs,
                   std::vector<ItemDef*> itemDefs);

    inline Block* getBlockDef(blockid_t id) const {
        if (id >= blockDefs.size())
            return nullptr;
        return blockDefs[id];
    }

    inline ItemDef* getItemDef(itemid_t id) const {
        if (id >= itemDefs.size())
            return nullptr;
        return itemDefs[id];
    }

    inline size_t countBlockDefs() const {
        return blockDefs.size();
    }

    inline size_t countItemDefs() const {
        return itemDefs.size();
    }

    // use this for critical spots to prevent range check overhead
    const Block* const* getBlockDefs() const {
        return blockDefs.data();
    }

    const ItemDef* const* getItemDefs() const {
        return itemDefs.data();
    }
};

/* Content is a definitions repository */
class Content {
    std::unordered_map<std::string, Block*> blockDefs;
    std::unordered_map<std::string, ItemDef*> itemDefs;
public:
    ContentIndices* const indices;
    DrawGroups* const drawGroups;

    Content(ContentIndices* indices, DrawGroups* drawGroups,
            std::unordered_map<std::string, Block*> blockDefs);
    ~Content();
    
    Block* findBlock(std::string id) const;
    Block* requireBlock(std::string id) const;

    ItemDef* findItem(std::string id) const;
    ItemDef* requireItem(std::string id) const;
};

#endif // CONTENT_CONTENT_H_