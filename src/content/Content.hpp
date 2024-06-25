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

template<class T>
class ContentUnitIndices {
    std::vector<T*> defs;
public:
    ContentUnitIndices(std::vector<T*> defs) : defs(std::move(defs)) {}

    inline T* get(blockid_t id) const {
        if (id >= defs.size()) {
            return nullptr;
        }
        return defs[id];
    }

    inline size_t count() const {
        return defs.size();
    }

    inline const T* const* getDefs() const {
        return defs.data();
    }
};

/// @brief Runtime defs cache: indices
class ContentIndices {
public:
    ContentUnitIndices<Block> blocks;
    ContentUnitIndices<ItemDef> items;

    ContentIndices(
        std::vector<Block*> blockDefs, 
        std::vector<ItemDef*> itemDefs
    );
};

template<class T>
class ContentUnitDefs {
    std::unordered_map<std::string, std::unique_ptr<T>> defs;
public:
    ContentUnitDefs(std::unordered_map<std::string, std::unique_ptr<T>> defs) 
    : defs(std::move(defs)) {
    }

    T* find(const std::string& id) const {
        const auto& found = defs.find(id);
        if (found == defs.end()) {
            return nullptr;
        }
        return found->second.get();
    }
    T& require(const std::string& id) const {
        const auto& found = defs.find(id);
        if (found == defs.end()) {
            throw std::runtime_error("missing content unit "+id);
        }
        return *found->second;
    }
};

/// @brief Content is a definitions repository
class Content {
    std::unique_ptr<ContentIndices> indices;
    std::unordered_map<std::string, std::unique_ptr<ContentPackRuntime>> packs;
    std::unordered_map<std::string, std::unique_ptr<BlockMaterial>> blockMaterials;
public:
    ContentUnitDefs<Block> blocks;
    ContentUnitDefs<ItemDef> items;
    std::unique_ptr<DrawGroups> const drawGroups;

    Content(
        std::unique_ptr<ContentIndices> indices, 
        std::unique_ptr<DrawGroups> drawGroups,
        ContentUnitDefs<Block> blocks,
        ContentUnitDefs<ItemDef> items,
        std::unordered_map<std::string, std::unique_ptr<ContentPackRuntime>> packs,
        std::unordered_map<std::string, std::unique_ptr<BlockMaterial>> blockMaterials
    );
    ~Content();

    inline ContentIndices* getIndices() const {
        return indices.get();
    }

    const BlockMaterial* findBlockMaterial(const std::string& id) const;
    const ContentPackRuntime* getPackRuntime(const std::string& id) const;

    const std::unordered_map<std::string, std::unique_ptr<BlockMaterial>>& getBlockMaterials() const;
    const std::unordered_map<std::string, std::unique_ptr<ContentPackRuntime>>& getPacks() const;
};

#endif // CONTENT_CONTENT_HPP_
