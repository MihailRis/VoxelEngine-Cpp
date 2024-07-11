#ifndef CONTENT_CONTENT_BUILDER_HPP_
#define CONTENT_CONTENT_BUILDER_HPP_

#include "../items/ItemDef.hpp"
#include "../voxels/Block.hpp"
#include "../objects/EntityDef.hpp"
#include "../content/Content.hpp"
#include "../content/ContentPack.hpp"

#include <memory>
#include <vector>
#include <unordered_map>

template<class T>
class ContentUnitBuilder {
    std::unordered_map<std::string, contenttype>& allNames;
    contenttype type;

    void checkIdentifier(const std::string& id) {
        const auto& found = allNames.find(id);
        if (found != allNames.end()) {
            throw namereuse_error("name "+id+" is already used", found->second);
        }
    }
public:
    UptrsMap<std::string, T> defs;
    std::vector<std::string> names;
    
    ContentUnitBuilder(
        std::unordered_map<std::string, contenttype>& allNames,
        contenttype type
    ) : allNames(allNames), type(type) {}

    T& create(const std::string& id) {
        auto found = defs.find(id);
        if (found != defs.end()) {
            return *found->second;
        }
        checkIdentifier(id);
        allNames[id] = type;
        names.push_back(id);
        defs[id] = std::make_unique<T>(id);
        return *defs[id];
    }

    auto build() {
        return std::move(defs);
    }
};

class ContentBuilder {
    UptrsMap<std::string, BlockMaterial> blockMaterials;
    UptrsMap<std::string, rigging::SkeletonConfig> skeletons;
    UptrsMap<std::string, ContentPackRuntime> packs;
    std::unordered_map<std::string, contenttype> allNames;
public:
    ContentUnitBuilder<Block> blocks {allNames, contenttype::block};
    ContentUnitBuilder<ItemDef> items {allNames, contenttype::item};
    ContentUnitBuilder<EntityDef> entities {allNames, contenttype::entity};
    ResourceIndicesSet resourceIndices {};

    ~ContentBuilder();

    void add(std::unique_ptr<ContentPackRuntime> pack);
    void add(std::unique_ptr<rigging::SkeletonConfig> skeleton);

    BlockMaterial& createBlockMaterial(const std::string& id);

    std::unique_ptr<Content> build();
};

#endif // CONTENT_CONTENT_BUILDER_HPP_
