#pragma once

#include <memory>
#include <vector>

#include "interfaces/Serializable.hpp"
#include "typedefs.hpp"
#include "ItemStack.hpp"

class ContentReport;
class ContentIndices;

class Inventory : public Serializable {
    int64_t id;
    std::vector<ItemStack> slots;
public:
    Inventory() = default;

    Inventory(int64_t id, size_t size);

    Inventory(const Inventory& orig);

    ItemStack& getSlot(size_t index);
    size_t findEmptySlot(size_t begin = 0, size_t end = -1) const;
    size_t findSlotByItem(
        itemid_t id, size_t begin = 0, size_t end = -1, size_t minCount = 1
    );

    inline size_t size() const {
        return slots.size();
    }

    void move(
        ItemStack& item,
        const ContentIndices* indices,
        size_t begin = 0,
        size_t end = -1
    );

    void resize(uint newSize);

    void deserialize(const dv::value& src) override;

    dv::value serialize() const override;

    void convert(const ContentReport* report);
    static void convert(dv::value& data, const ContentReport* report);

    inline void setId(int64_t id) {
        this->id = id;
    }

    inline int64_t getId() const {
        return id;
    }

    inline bool isVirtual() const {
        return id < 0;
    }

    static const size_t npos;
};
