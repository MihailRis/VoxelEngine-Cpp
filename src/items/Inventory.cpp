#include "Inventory.hpp"

#include "content/ContentReport.hpp"
#include "data/dynamic.hpp"

Inventory::Inventory(int64_t id, size_t size) : id(id), slots(size) {
}

Inventory::Inventory(const Inventory& orig) {
    this->slots = orig.slots;
}

ItemStack& Inventory::getSlot(size_t index) {
    return slots.at(index);
}

size_t Inventory::findEmptySlot(size_t begin, size_t end) const {
    end = std::min(slots.size(), end);
    for (size_t i = begin; i < end; i++) {
        if (slots[i].isEmpty()) {
            return i;
        }
    }
    return npos;
}

size_t Inventory::findSlotByItem(itemid_t id, size_t begin, size_t end) {
    end = std::min(slots.size(), end);
    for (size_t i = begin; i < end; i++) {
        if (slots[i].getItemId() == id) {
            return i;
        }
    }
    return npos;
}

void Inventory::move(
    ItemStack& item, const ContentIndices* indices, size_t begin, size_t end
) {
    end = std::min(slots.size(), end);
    for (size_t i = begin; i < end && !item.isEmpty(); i++) {
        ItemStack& slot = slots[i];
        if (slot.accepts(item)) {
            slot.move(item, indices);
        }
    }
}

void Inventory::deserialize(dynamic::Map* src) {
    id = src->get("id", 1);
    auto slotsarr = src->list("slots");
    size_t slotscount = slotsarr->size();
    while (slots.size() < slotscount) {
        slots.emplace_back();
    }
    for (size_t i = 0; i < slotscount; i++) {
        auto item = slotsarr->map(i);
        itemid_t id = item->get("id", ITEM_EMPTY);
        itemcount_t count = item->get("count", 0);
        auto& slot = slots[i];
        slot.set(ItemStack(id, count));
    }
}

std::unique_ptr<dynamic::Map> Inventory::serialize() const {
    auto map = std::make_unique<dynamic::Map>();
    map->put("id", id);

    auto& slotsarr = map->putList("slots");
    for (size_t i = 0; i < slots.size(); i++) {
        auto& item = slots[i];
        itemid_t id = item.getItemId();
        itemcount_t count = item.getCount();

        auto& slotmap = slotsarr.putMap();
        slotmap.put("id", id);
        if (count) {
            slotmap.put("count", count);
        }
    }
    return map;
}

void Inventory::convert(const ContentReport* report) {
    for (auto& slot : slots) {
        itemid_t id = slot.getItemId();
        itemid_t replacement = report->items.getId(id);
        slot.set(ItemStack(replacement, slot.getCount()));
    }
}

// TODO: remove
void Inventory::convert(dynamic::Map* data, const ContentReport* report) {
    auto slotsarr = data->list("slots");
    for (size_t i = 0; i < slotsarr->size(); i++) {
        auto item = slotsarr->map(i);
        itemid_t id = item->get("id", ITEM_EMPTY);
        itemid_t replacement = report->items.getId(id);
        item->put("id", replacement);
        if (replacement == 0 && item->has("count")) {
            item->remove("count");
        }
    }
}

const size_t Inventory::npos = -1;
