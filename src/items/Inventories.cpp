#include "Inventories.hpp"

#include <algorithm>

#include <world/Level.hpp>
#include <world/World.hpp>

Inventories::Inventories(Level& level) : level(level) {
}

Inventories::~Inventories() = default;

std::shared_ptr<Inventory> Inventories::create(size_t size) {
    int64_t id = level.getWorld()->getNextInventoryId();
    auto inv = std::make_shared<Inventory>(id, size);
    store(inv);
    return inv;
}

std::shared_ptr<Inventory> Inventories::createVirtual(size_t size) {
    int64_t id;
    do {
        id = -std::max<int64_t>(1LL, std::llabs(random.rand64()));
    } while (map.find(id) != map.end());

    auto inv = std::make_shared<Inventory>(id, size);
    store(inv);
    return std::shared_ptr<Inventory>(inv.get(), [this](Inventory* ptr) {
        remove(ptr->getId());
    });
}

void Inventories::store(const std::shared_ptr<Inventory>& inv) {
    map[inv->getId()] = inv;
}

void Inventories::remove(int64_t id) {
    map.erase(id);
}

std::shared_ptr<Inventory> Inventories::get(int64_t id) {
    auto found = map.find(id);
    if (found == map.end()) return nullptr;
    return found->second;
}

std::shared_ptr<Inventory> Inventories::clone(int64_t id) {
    auto original = get(id);
    if (original == nullptr) return nullptr;
    auto clone = std::make_shared<Inventory>(*original);
    clone->setId(level.getWorld()->getNextInventoryId());
    store(clone);
    return clone;
}

const inventories_map& Inventories::getMap() const {
    return map;
}
