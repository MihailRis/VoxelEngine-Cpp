#include "Inventory.h"

#include "../content/ContentLUT.h"

Inventory::Inventory(size_t size) : slots(size) {
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
    ItemStack& item, 
    const ContentIndices* indices, 
    size_t begin, 
    size_t end) 
{
    end = std::min(slots.size(), end);
    for (size_t i = begin; i < end && !item.isEmpty(); i++) {
        ItemStack& slot = slots[i];
        if (slot.accepts(item)) {
            slot.move(item, indices);
        }
    }
}

void Inventory::read(const dynamic::Map* src) {
    auto slotsarr = src->list("slots");
    size_t slotscount = std::min(slotsarr->size(), slots.size());
    for (size_t i = 0; i < slotscount; i++) {
        auto item = slotsarr->map(i);
        itemid_t id = item->getInt("id", ITEM_EMPTY);
        itemcount_t count = item->getInt("count", 0);
        auto& slot = slots[i];
        slot.set(ItemStack(id, count)); 
    }
}

std::unique_ptr<dynamic::Map> Inventory::write() const {
    auto map = std::make_unique<dynamic::Map>();
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

void Inventory::convert(dynamic::Map* data, const ContentLUT* lut) {
    auto slotsarr = data->list("slots");
    for (size_t i = 0; i < slotsarr->size(); i++) {
        auto item = slotsarr->map(i);
        itemid_t id = item->getInt("id", ITEM_EMPTY);
        item->put("id", lut->getItemId(id));
    }
}

const size_t Inventory::npos = -1;
