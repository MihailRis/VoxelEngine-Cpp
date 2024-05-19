#ifndef CONTENT_CONTENT_BUILDER_HPP_
#define CONTENT_CONTENT_BUILDER_HPP_

#include "../items/ItemDef.hpp"
#include "../voxels/Block.hpp"
#include "../content/Content.hpp"
#include "../content/ContentPack.hpp"

#include <memory>
#include <vector>
#include <unordered_map>

class ContentBuilder {
    std::unordered_map<std::string, std::unique_ptr<Block>> blockDefs;
    std::vector<std::string> blockIds;

    std::unordered_map<std::string, std::unique_ptr<ItemDef>> itemDefs;
    std::vector<std::string> itemIds;

    std::unordered_map<std::string, std::unique_ptr<BlockMaterial>> blockMaterials;
    std::unordered_map<std::string, std::unique_ptr<ContentPackRuntime>> packs;
public:
    ~ContentBuilder();

    void add(std::unique_ptr<ContentPackRuntime> pack);

    Block& createBlock(std::string id);
    ItemDef& createItem(std::string id);
    BlockMaterial& createBlockMaterial(std::string id);

    void checkIdentifier(std::string id);
    contenttype checkContentType(std::string id);

    std::unique_ptr<Content> build();
};

#endif // CONTENT_CONTENT_BUILDER_HPP_
