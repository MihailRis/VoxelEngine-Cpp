#include "Inventories.h"

#include <algorithm>

#include "../world/Level.h"
#include "../world/World.h"

Inventories::Inventories(Level& level) : level(level) {
}

Inventories::~Inventories() {
}

std::shared_ptr<Inventory> Inventories::create(size_t size) {
    int64_t id = level.getWorld()->getNextInventoryId();
    auto inv = std::make_shared<Inventory>(id, size);
    store(inv);
    return inv;
}

std::shared_ptr<Inventory> Inventories::createVirtual(size_t size) {
    int64_t id;
    do {
        id = -std::max(1LL, std::abs(random.rand64()));
    } while (map.find(id) != map.end());

    auto inv = std::make_shared<Inventory>(id, size);
    store(inv);
    return inv;
}

void Inventories::store(std::shared_ptr<Inventory> inv) {
    map[inv->getId()] = inv;
}

std::shared_ptr<Inventory> Inventories::get(int64_t id) {
    auto found = map.find(id);
    if (found == map.end())
        return nullptr;
    return found->second;
}

const inventories_map& Inventories::getMap() const {
    return map;
}
