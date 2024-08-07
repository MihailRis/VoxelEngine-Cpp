#include "ContentBuilder.hpp"

#include <objects/rigging.hpp>

ContentBuilder::~ContentBuilder() = default;

void ContentBuilder::add(std::unique_ptr<ContentPackRuntime> pack) {
    packs[pack->getId()] = std::move(pack);
}

void ContentBuilder::add(std::unique_ptr<rigging::SkeletonConfig> skeleton) {
    skeletons[skeleton->getName()] = std::move(skeleton);
}

BlockMaterial& ContentBuilder::createBlockMaterial(const std::string& id) {
    blockMaterials[id] = std::make_unique<BlockMaterial>();
    auto& material = *blockMaterials[id];
    material.name = id;
    return material;
}

std::unique_ptr<Content> ContentBuilder::build() {
    std::vector<Block*> blockDefsIndices;
    auto groups = std::make_unique<DrawGroups>();
    for (const std::string& name : blocks.names) {
        Block& def = *blocks.defs[name];

        // Generating runtime info
        def.rt.id = blockDefsIndices.size();
        def.rt.emissive = *reinterpret_cast<uint32_t*>(def.emission);
        def.rt.solid = def.model == BlockModel::block;
        def.rt.extended = def.size.x > 1 || def.size.y > 1 || def.size.z > 1;

        if (def.rotatable) {
            for (uint i = 0; i < BlockRotProfile::MAX_COUNT; i++) {
                def.rt.hitboxes[i].reserve(def.hitboxes.size());
                for (AABB aabb : def.hitboxes) {
                    def.rotations.variants[i].transform(aabb);
                    def.rt.hitboxes[i].push_back(aabb);
                }
            }
        }

        blockDefsIndices.push_back(&def);
        groups->insert(def.drawGroup);
    }

    std::vector<ItemDef*> itemDefsIndices;
    for (const std::string& name : items.names) {
        ItemDef& def = *items.defs[name];

        // Generating runtime info
        def.rt.id = itemDefsIndices.size();
        def.rt.emissive = *reinterpret_cast<uint32_t*>(def.emission);
        itemDefsIndices.push_back(&def);
    }

    std::vector<EntityDef*> entityDefsIndices;
    for (const std::string& name : entities.names) {
        EntityDef& def = *entities.defs[name];

        // Generating runtime info
        def.rt.id = entityDefsIndices.size();
        entityDefsIndices.push_back(&def);
    }

    auto content = std::make_unique<Content>(
        std::make_unique<ContentIndices>(
            blockDefsIndices, itemDefsIndices, entityDefsIndices
        ),
        std::move(groups),
        blocks.build(),
        items.build(),
        entities.build(),
        std::move(packs),
        std::move(blockMaterials),
        std::move(skeletons),
        std::move(resourceIndices)
    );

    // Now, it's time to resolve foreign keys
    for (Block* def : blockDefsIndices) {
        def->rt.pickingItem = content->items.require(def->pickingItem).rt.id;
    }

    for (ItemDef* def : itemDefsIndices) {
        def->rt.placingBlock = content->blocks.require(def->placingBlock).rt.id;
    }

    return content;
}
