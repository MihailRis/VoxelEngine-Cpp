#ifndef CONTENT_CONTENT_H_
#define CONTENT_CONTENT_H_

#include <string>
#include <vector>
#include <unordered_map>
#include <set>
#include "../typedefs.h"

typedef std::set<unsigned char> DrawGroups;

class Block;
class Item;
class Content;

class ContentBuilder {
    std::unordered_map<std::string, Block*> blockDefs;
    std::unordered_map<std::string, Item*> itemDefs;
    std::vector<std::string> blockIds;
    std::vector<std::string> itemIds;
public:
    void add(Block* def);
    void add(Item* def);

    Content* build();
};

/* Runtime defs cache: indices */
class ContentIndices {
    // blockDefs must be a plain vector with block id used as index
    std::vector<Block*> blockDefs;
    std::vector<Item*> itemDefs;
public:
    ContentIndices(std::vector<Block*> blockDefs, std::vector<Item*> itemDefs);

    inline Block* getBlockDef(blockid_t id) const {
        if (id >= blockDefs.size())
            return nullptr;
        return blockDefs[id];
    }

    inline size_t countBlockDefs() const {
        return blockDefs.size();
    }

    // use this for critical spots to prevent range check overhead
    const Block* const* getBlockDefs() const {
        return blockDefs.data();
    }

    inline Item* getItemDef(itemid_t id) const {
        if (id >= itemDefs.size())
            return nullptr;
        return itemDefs[id];
    }

    inline size_t countItemDefs() const {
        return itemDefs.size();
    }

    const Item* const* getItemDefs() const {
        return itemDefs.data();
    }
};

/* Content is a definitions repository */
class Content {
    std::unordered_map<std::string, Block*> blockDefs;
    std::unordered_map<std::string, Item*> itemDefs;
public:
    ContentIndices* const indices;
    DrawGroups* const drawGroups;

    Content(ContentIndices* indices, DrawGroups* drawGroups,
            std::unordered_map<std::string, Block*> blockDefs,
            std::unordered_map<std::string, Item*> itemDefs);
    ~Content();
    
    Block* findBlock(std::string id) const;
    Block* requireBlock(std::string id) const;

    Item* findItem(std::string id) const;
    Item* requireItem(std::string id) const;
};

#endif // CONTENT_CONTENT_H_