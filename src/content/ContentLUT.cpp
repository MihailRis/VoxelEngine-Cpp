#include "ContentLUT.hpp"

#include "Content.hpp"
#include "../constants.hpp"
#include "../files/files.hpp"
#include "../coders/json.hpp"
#include "../voxels/Block.hpp"
#include "../items/ItemDef.hpp"
#include "../data/dynamic.hpp"

#include <memory>

template<typename I, class T>
static void init(std::vector<I>& ids, std::vector<std::string>& names, 
                 const ContentUnitIndices<T>& indices, size_t count) {
    for (size_t i = 0; i < count; i++) {
        ids.push_back(i);
    }
    for (size_t i = 0; i < indices.count(); i++) {
        names.push_back(indices.get(i)->name);
    }
    for (size_t i = indices.count(); i < count; i++) {
        names.emplace_back("");
    }
}

ContentLUT::ContentLUT(const Content* content, size_t blocksCount, size_t itemsCount) {
    auto* indices = content->getIndices();
    init(blocks, blockNames, indices->blocks, blocksCount);
    init(items, itemNames, indices->items, itemsCount);
}

template<class T>
static void setup_lut(ContentLUT* lut, const ContentUnitDefs<T>& defs, dynamic::List* list) {
    if (list) {
        for (size_t i = 0; i < list->size(); i++) {
            std::string name = list->str(i);
            if (auto def = defs.find(name)) {
                lut->setBlock(i, name, def->rt.id);
            } else {
                lut->setBlock(i, name, BLOCK_VOID);   
            }
        }
    }
}

template<class T> static constexpr size_t get_entries_count(
    const ContentUnitIndices<T>& indices, dynamic::List* list) {
    return list ? std::max(list->size(), indices.count()) : indices.count();
}

std::shared_ptr<ContentLUT> ContentLUT::create(
    const fs::path& filename, 
    const Content* content
) {
    auto root = files::read_json(filename);
    auto blocklist = root->list("blocks");
    auto itemlist = root->list("items");

    auto* indices = content->getIndices();
    size_t blocks_c = get_entries_count(indices->blocks, blocklist);
    size_t items_c = get_entries_count(indices->items, itemlist);

    auto lut = std::make_shared<ContentLUT>(content, blocks_c, items_c);

    setup_lut(lut.get(), content->blocks, blocklist);
    setup_lut(lut.get(), content->items, itemlist);

    if (lut->hasContentReorder() || lut->hasMissingContent()) {
        return lut;
    } else {
        return nullptr;
    }
}

std::vector<contententry> ContentLUT::getMissingContent() const {
    std::vector<contententry> entries;
    for (size_t i = 0; i < blocks.size(); i++) {
        if (blocks[i] == BLOCK_VOID) {
            auto& name = blockNames[i];
            entries.push_back(contententry {contenttype::block, name});
        }
    }
    for (size_t i = 0; i < items.size(); i++) {
        if (items[i] == ITEM_VOID) {
            auto& name = itemNames[i];
            entries.push_back(contententry {contenttype::item, name});
        }
    }
    return entries;
}
