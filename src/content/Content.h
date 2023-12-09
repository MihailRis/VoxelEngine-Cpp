#ifndef CONTENT_CONTENT_H_
#define CONTENT_CONTENT_H_

#include <string>
#include <vector>
#include <unordered_map>
#include <set>
#include "../typedefs.h"

typedef std::set<unsigned char> DrawGroups;

class Block;
class Content;

class ContentBuilder {
    std::unordered_map<std::string, Block*> blockDefs;
    std::vector<std::string> blockIds;
public:
    void add(Block* def);

    Content* build();
};

/* Runtime defs cache: indices */
class ContentIndices {
    // blockDefs must be a plain vector with block id used as index
    std::vector<Block*> blockDefs;
public:
    ContentIndices(const std::vector<Block*>& blockDefs);

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
};

/* Content is a definitions repository */
class Content {
    std::unordered_map<std::string, Block*> blockDefs;
public:
    ContentIndices* const indices;
    DrawGroups* const drawGroups;

    Content(ContentIndices* indices, DrawGroups* drawGroups,
            const std::unordered_map<std::string, Block*>& blockDefs);
    ~Content();
    
    Block* findBlock(const std::string& id) const;
    Block* requireBlock(const std::string& id) const;
};

#endif // CONTENT_CONTENT_H_