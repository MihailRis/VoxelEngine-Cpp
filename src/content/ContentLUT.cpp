#include "ContentLUT.hpp"

#include <memory>

#include "coders/json.hpp"
#include "constants.hpp"
#include "files/files.hpp"
#include "items/ItemDef.hpp"
#include "voxels/Block.hpp"
#include "world/World.hpp"
#include "files/WorldFiles.hpp"
#include "Content.hpp"

ContentLUT::ContentLUT(
    const ContentIndices* indices, 
    size_t blocksCount, 
    size_t itemsCount
)
    : blocks(blocksCount, indices->blocks, BLOCK_VOID, ContentType::BLOCK),
      items(itemsCount, indices->items, ITEM_VOID, ContentType::ITEM)
{}

template <class T>
static constexpr size_t get_entries_count(
    const ContentUnitIndices<T>& indices, const dynamic::List_sptr& list
) {
    return list ? std::max(list->size(), indices.count()) : indices.count();
}

std::shared_ptr<ContentLUT> ContentLUT::create(
    const std::shared_ptr<WorldFiles>& worldFiles,
    const fs::path& filename, 
    const Content* content
) {
    auto worldInfo = worldFiles->readWorldInfo();
    if (!worldInfo.has_value()) {
        return nullptr;
    }

    auto root = files::read_json(filename);
    auto blocklist = root->list("blocks");
    auto itemlist = root->list("items");

    auto* indices = content->getIndices();
    size_t blocks_c = get_entries_count(indices->blocks, blocklist);
    size_t items_c = get_entries_count(indices->items, itemlist);

    auto lut = std::make_shared<ContentLUT>(indices, blocks_c, items_c);

    lut->blocks.setup(blocklist.get(), content->blocks);
    lut->items.setup(itemlist.get(), content->items);
    lut->buildIssues();

    if (lut->hasContentReorder() || lut->hasMissingContent()) {
        return lut;
    } else {
        return nullptr;
    }
}

template<class T, class U>
static void build_issues(
    std::vector<ContentIssue>& issues,
    const ContentUnitLUT<T, U>& lut
) {
    auto type = lut.getContentType();
    if (lut.hasContentReorder()) {
        issues.push_back(ContentIssue {ContentIssueType::REORDER, type});
    }
    if (lut.hasMissingContent()) {
        issues.push_back(ContentIssue {ContentIssueType::MISSING, type});
    }
}

void ContentLUT::buildIssues() {
    build_issues(issues, blocks);
    build_issues(issues, items);
}

const std::vector<ContentIssue>& ContentLUT::getIssues() const {
    return issues;
}

std::vector<ContentEntry> ContentLUT::getMissingContent() const {
    std::vector<ContentEntry> entries;
    blocks.getMissingContent(entries);
    items.getMissingContent(entries);
    return entries;
}
