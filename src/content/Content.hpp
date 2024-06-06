#ifndef CONTENT_CONTENT_HPP_
#define CONTENT_CONTENT_HPP_

#include "../typedefs.hpp"

#include <string>
#include <vector>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <set>

using DrawGroups = std::set<ubyte>;

class Block;
struct BlockMaterial;
class ItemDef;
class Content;
class ContentPackRuntime;

enum class contenttype {
    none, block, item
};

inline const char* contenttype_name(contenttype type) {
    switch (type) {
        case contenttype::none: return "none";
        case contenttype::block: return "block";
        case contenttype::item: return "item";
        default:
            return "unknown";
    }
}

class namereuse_error: public std::runtime_error {
    contenttype type;
public:
    namereuse_error(const std::string& msg, contenttype type)
        : std::runtime_error(msg), type(type) {}

    inline contenttype getType() const {
        return type;
    }
};

/// @brief Runtime defs cache: indices
class ContentIndices {
    std::vector<Block*> blockDefs;
    std::vector<ItemDef*> itemDefs;
public:
    ContentIndices(
        std::vector<Block*> blockDefs, 
        std::vector<ItemDef*> itemDefs
    );

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
    std::unordered_map<std::string, std::unique_ptr<Block>> blockDefs;
    std::unordered_map<std::string, std::unique_ptr<ItemDef>> itemDefs;
    std::unique_ptr<ContentIndices> indices;
    std::unordered_map<std::string, std::unique_ptr<ContentPackRuntime>> packs;
    std::unordered_map<std::string, std::unique_ptr<BlockMaterial>> blockMaterials;
public:
    std::unique_ptr<DrawGroups> const drawGroups;

    Content(
        std::unique_ptr<ContentIndices> indices, 
        std::unique_ptr<DrawGroups> drawGroups,
        std::unordered_map<std::string, std::unique_ptr<Block>> blockDefs,
        std::unordered_map<std::string, std::unique_ptr<ItemDef>> itemDefs,
        std::unordered_map<std::string, std::unique_ptr<ContentPackRuntime>> packs,
        std::unordered_map<std::string, std::unique_ptr<BlockMaterial>> blockMaterials
    );
    ~Content();

    inline ContentIndices* getIndices() const {
        return indices.get();
    }
    
    Block* findBlock(std::string id) const;
    Block& requireBlock(std::string id) const;

    ItemDef* findItem(std::string id) const;
    ItemDef& requireItem(std::string id) const;

    const BlockMaterial* findBlockMaterial(std::string id) const;

    const ContentPackRuntime* getPackRuntime(std::string id) const;

    const std::unordered_map<std::string, std::unique_ptr<BlockMaterial>>& getBlockMaterials() const;
    const std::unordered_map<std::string, std::unique_ptr<ContentPackRuntime>>& getPacks() const;
};

#endif // CONTENT_CONTENT_HPP_
