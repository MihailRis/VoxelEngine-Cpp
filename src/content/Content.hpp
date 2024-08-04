#ifndef CONTENT_CONTENT_HPP_
#define CONTENT_CONTENT_HPP_

#include <memory>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "../data/dynamic_fwd.hpp"
#include "content_fwd.hpp"

using DrawGroups = std::set<ubyte>;
template <class K, class V>
using UptrsMap = std::unordered_map<K, std::unique_ptr<V>>;

class Block;
struct BlockMaterial;
struct ItemDef;
struct EntityDef;

namespace rigging {
    class SkeletonConfig;
}

constexpr const char* contenttype_name(contenttype type) {
    switch (type) {
        case contenttype::none:
            return "none";
        case contenttype::block:
            return "block";
        case contenttype::item:
            return "item";
        case contenttype::entity:
            return "entity";
        default:
            return "unknown";
    }
}

class namereuse_error : public std::runtime_error {
    contenttype type;
public:
    namereuse_error(const std::string& msg, contenttype type)
        : std::runtime_error(msg), type(type) {
    }

    inline contenttype getType() const {
        return type;
    }
};

template <class T>
class ContentUnitIndices {
    std::vector<T*> defs;
public:
    ContentUnitIndices(std::vector<T*> defs) : defs(std::move(defs)) {
    }

    inline const T* get(blockid_t id) const {
        if (id >= defs.size()) {
            return nullptr;
        }
        return defs[id];
    }

    [[deprecated]]
    inline T* getWriteable(blockid_t id) const { // TODO: remove
        if (id >= defs.size()) {
            return nullptr;
        }
        return defs[id];
    }

    inline const T& require(blockid_t id) const {
        return *defs.at(id);
    }

    inline size_t count() const {
        return defs.size();
    }

    inline const auto& getIterable() const {
        return defs;
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

template <class T>
class ContentUnitDefs {
    UptrsMap<std::string, T> defs;
public:
    ContentUnitDefs(UptrsMap<std::string, T> defs) : defs(std::move(defs)) {
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
            throw std::runtime_error("missing content unit " + id);
        }
        return *found->second;
    }
};

class ResourceIndices {
    std::vector<std::string> names;
    std::unordered_map<std::string, size_t> indices;
    std::unique_ptr<std::vector<dynamic::Map_sptr>> savedData;
public:
    ResourceIndices()
        : savedData(std::make_unique<std::vector<dynamic::Map_sptr>>()) {
    }

    static constexpr size_t MISSING = SIZE_MAX;

    void add(std::string name, dynamic::Map_sptr map) {
        indices[name] = names.size();
        names.push_back(name);
        savedData->push_back(map);
    }

    const std::string& getName(size_t index) const {
        return names.at(index);
    }

    size_t indexOf(const std::string& name) const {
        const auto& found = indices.find(name);
        if (found != indices.end()) {
            return found->second;
        }
        return MISSING;
    }

    dynamic::Map_sptr getSavedData(size_t index) const {
        return savedData->at(index);
    }

    void saveData(size_t index, dynamic::Map_sptr map) const {
        savedData->at(index) = map;
    }

    size_t size() const {
        return names.size();
    }
};

constexpr const char* to_string(ResourceType type) {
    switch (type) {
        case ResourceType::CAMERA:
            return "camera";
        default:
            return "unknown";
    }
}

inline std::optional<ResourceType> ResourceType_from(std::string_view str) {
    if (str == "camera") {
        return ResourceType::CAMERA;
    }
    return std::nullopt;
}

using ResourceIndicesSet = ResourceIndices[RESOURCE_TYPES_COUNT];

/// @brief Content is a definitions repository
class Content {
    std::unique_ptr<ContentIndices> indices;
    UptrsMap<std::string, ContentPackRuntime> packs;
    UptrsMap<std::string, BlockMaterial> blockMaterials;
    UptrsMap<std::string, rigging::SkeletonConfig> skeletons;
public:
    ContentUnitDefs<Block> blocks;
    ContentUnitDefs<ItemDef> items;
    ContentUnitDefs<EntityDef> entities;
    std::unique_ptr<DrawGroups> const drawGroups;
    ResourceIndicesSet resourceIndices {};

    Content(
        std::unique_ptr<ContentIndices> indices,
        std::unique_ptr<DrawGroups> drawGroups,
        ContentUnitDefs<Block> blocks,
        ContentUnitDefs<ItemDef> items,
        ContentUnitDefs<EntityDef> entities,
        UptrsMap<std::string, ContentPackRuntime> packs,
        UptrsMap<std::string, BlockMaterial> blockMaterials,
        UptrsMap<std::string, rigging::SkeletonConfig> skeletons,
        ResourceIndicesSet resourceIndices
    );
    ~Content();

    inline ContentIndices* getIndices() const {
        return indices.get();
    }

    inline const ResourceIndices& getIndices(ResourceType type) const {
        return resourceIndices[static_cast<size_t>(type)];
    }

    const rigging::SkeletonConfig* getSkeleton(const std::string& id) const;
    const BlockMaterial* findBlockMaterial(const std::string& id) const;
    const ContentPackRuntime* getPackRuntime(const std::string& id) const;

    const UptrsMap<std::string, BlockMaterial>& getBlockMaterials() const;
    const UptrsMap<std::string, ContentPackRuntime>& getPacks() const;
    const UptrsMap<std::string, rigging::SkeletonConfig>& getSkeletons() const;
};

#endif  // CONTENT_CONTENT_HPP_
