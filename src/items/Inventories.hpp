#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "maths/util.hpp"
#include "Inventory.hpp"

class Level;

using inventories_map = std::unordered_map<int64_t, std::shared_ptr<Inventory>>;

/* Inventories runtime storage */
class Inventories {
    Level& level;
    inventories_map map;
    util::PseudoRandom random;
public:
    Inventories(Level& level);
    ~Inventories();

    /* Create new inventory with new id */
    std::shared_ptr<Inventory> create(size_t size);

    /* Create runtime-only inventory (has negative id) */
    std::shared_ptr<Inventory> createVirtual(size_t size);

    /* Store inventory */
    void store(const std::shared_ptr<Inventory>& inv);

    /* Remove inventory from map */
    void remove(int64_t id);

    /* Get inventory by id (works with both real and virtual)*/
    std::shared_ptr<Inventory> get(int64_t id);

    std::shared_ptr<Inventory> clone(int64_t id);

    const inventories_map& getMap() const;
};

