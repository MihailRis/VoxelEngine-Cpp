#include "Players.hpp"

#include "Player.hpp"
#include "items/Inventories.hpp"
#include "world/Level.hpp"
#include "world/World.hpp"

Players::Players(Level* level) : level(level) {}

void Players::addPlayer(std::unique_ptr<Player> player) {
    players[player->getId()] = std::move(player);
}

Player* Players::getPlayer(int64_t id) const {
    const auto& found = players.find(id);
    if (found == players.end()) {
        return nullptr;
    }
    return found->second.get();
}

Player* Players::create() {
    auto playerPtr = std::make_unique<Player>(
        level,
        level->getWorld()->getInfo().nextPlayerId++,
        glm::vec3(0, DEF_PLAYER_Y, 0),
        DEF_PLAYER_SPEED,
        level->inventories->create(DEF_PLAYER_INVENTORY_SIZE),
        0
    );
    auto player = playerPtr.get();
    addPlayer(std::move(playerPtr));

    level->inventories->store(player->getInventory());
    return player;
}

dv::value Players::serialize() const {
    auto root = dv::object();
    auto& list = root.list("players");

    for (const auto& [id, player] : players) {
        list.add(player->serialize());
    }
    return root;
}

void Players::deserialize(const dv::value& src) {
    players.clear();

    const auto& players = src["players"];
    for (auto& playerMap : players) {
        auto playerPtr = std::make_unique<Player>(
            level,
            0,
            glm::vec3(0, DEF_PLAYER_Y, 0),
            DEF_PLAYER_SPEED,
            level->inventories->create(DEF_PLAYER_INVENTORY_SIZE),
            0
        );
        auto player = playerPtr.get();
        player->deserialize(playerMap);
        addPlayer(std::move(playerPtr));
        auto& inventory = player->getInventory();
        // invalid inventory id pre 0.25
        if (inventory->getId() == 0) {
            inventory->setId(level->getWorld()->getNextInventoryId());
        }
        level->inventories->store(player->getInventory());
    }
}
