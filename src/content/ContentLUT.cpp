#include "ContentLUT.hpp"

#include "Content.hpp"
#include "../constants.hpp"
#include "../files/files.hpp"
#include "../coders/json.hpp"
#include "../voxels/Block.hpp"
#include "../items/ItemDef.hpp"
#include <memory>

ContentLUT::ContentLUT(const ContentIndices* indices, size_t blocksCount, size_t itemsCount) 
  : blocks(blocksCount, indices->blocks, BLOCK_VOID, contenttype::block),
    items(itemsCount, indices->items, ITEM_VOID, contenttype::item)
{}

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

    auto lut = std::make_shared<ContentLUT>(indices, blocks_c, items_c);

    lut->blocks.setup(blocklist, content->blocks);
    lut->items.setup(itemlist, content->items);

    if (lut->hasContentReorder() || lut->hasMissingContent()) {
        return lut;
    } else {
        return nullptr;
    }
}

std::vector<contententry> ContentLUT::getMissingContent() const {
    std::vector<contententry> entries;
    blocks.getMissingContent(entries);
    items.getMissingContent(entries);
    return entries;
}
