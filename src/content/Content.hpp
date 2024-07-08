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
template<class K, class V>
using UptrsMap = std::unordered_map<K, std::unique_ptr<V>>;

class Block;
struct BlockMaterial;
struct ItemDef;
struct EntityDef;
class Content;
class ContentPackRuntime;

namespace rigging {
    class RigConfig;
}

enum class contenttype {
    none, block, item, entity
};

inline const char* contenttype_name(contenttype type) {
    switch (type) {
        case contenttype::none: return "none";
        case contenttype::block: return "block";
        case contenttype::item: return "item";
        case contenttype::entity: return "entity";
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
    ContentUnitIndices<EntityDef> entities;

    ContentIndices(
        ContentUnitIndices<Block> blocks,
        ContentUnitIndices<ItemDef> items,
        ContentUnitIndices<EntityDef> entities
    );
};

template<class T>
class ContentUnitDefs {
    UptrsMap<std::string, T> defs;
public:
    ContentUnitDefs(UptrsMap<std::string, T> defs) 
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
    UptrsMap<std::string, ContentPackRuntime> packs;
    UptrsMap<std::string, BlockMaterial> blockMaterials;
    UptrsMap<std::string, rigging::RigConfig> rigs;
public:
    ContentUnitDefs<Block> blocks;
    ContentUnitDefs<ItemDef> items;
    ContentUnitDefs<EntityDef> entities;
    std::unique_ptr<DrawGroups> const drawGroups;

    Content(
        std::unique_ptr<ContentIndices> indices, 
        std::unique_ptr<DrawGroups> drawGroups,
        ContentUnitDefs<Block> blocks,
        ContentUnitDefs<ItemDef> items,
        ContentUnitDefs<EntityDef> entities,
        UptrsMap<std::string, ContentPackRuntime> packs,
        UptrsMap<std::string, BlockMaterial> blockMaterials,
        UptrsMap<std::string, rigging::RigConfig> rigs
    );
    ~Content();

    inline ContentIndices* getIndices() const {
        return indices.get();
    }

    const rigging::RigConfig* getRig(const std::string& id) const;
    const BlockMaterial* findBlockMaterial(const std::string& id) const;
    const ContentPackRuntime* getPackRuntime(const std::string& id) const;

    const UptrsMap<std::string, BlockMaterial>& getBlockMaterials() const;
    const UptrsMap<std::string, ContentPackRuntime>& getPacks() const;
};

#endif // CONTENT_CONTENT_HPP_
