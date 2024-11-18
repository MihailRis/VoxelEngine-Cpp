#include "Inventory.hpp"

#include "content/ContentReport.hpp"

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

void Inventory::resize(uint newSize) {
    slots.resize(newSize);
}

void Inventory::deserialize(const dv::value& src) {
    id = src["id"].asInteger(1);
    auto& slotsarr = src["slots"];
    size_t slotscount = slotsarr.size();
    while (slots.size() < slotscount) {
        slots.emplace_back();
    }
    for (size_t i = 0; i < slotscount; i++) {
        auto& item = slotsarr[i];
        itemid_t id = item["id"].asInteger();
        itemcount_t count = 0;
        if (item.has("count")){
            count = item["count"].asInteger();
        }
        auto& slot = slots[i];
        slot.set(ItemStack(id, count));
    }
}

dv::value Inventory::serialize() const {
    auto map = dv::object();
    map["id"] = id;
    auto& slotsarr = map.list("slots");

    for (size_t i = 0; i < slots.size(); i++) {
        auto& item = slots[i];
        itemid_t id = item.getItemId();
        itemcount_t count = item.getCount();

        auto& slotmap = slotsarr.object();
        slotmap["id"] = id;
        if (count) {
            slotmap["count"] = count;
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

void Inventory::convert(dv::value& data, const ContentReport* report) {
    Inventory inventory;
    inventory.deserialize(data);
    inventory.convert(report);
    data = inventory.serialize();
}

const size_t Inventory::npos = -1;
