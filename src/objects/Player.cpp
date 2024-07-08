#include "Player.hpp"

#include "../content/ContentLUT.hpp"
#include "../physics/Hitbox.hpp"
#include "../physics/PhysicsSolver.hpp"
#include "../voxels/Chunks.hpp"
#include "../world/Level.hpp"
#include "../window/Events.hpp"
#include "../window/Camera.hpp"
#include "../items/Inventory.hpp"
#include "../objects/Entities.hpp"

#include <glm/glm.hpp>
#include <utility>

const float CROUCH_SPEED_MUL = 0.35f;
const float RUN_SPEED_MUL = 1.5f;
const float PLAYER_GROUND_DAMPING = 10.0f;
const float PLAYER_AIR_DAMPING = 7.0f;
const float FLIGHT_SPEED_MUL = 4.0f;
const float CHEAT_SPEED_MUL = 5.0f;
const float JUMP_FORCE = 8.0f;

Player::Player(Level* level, glm::vec3 position, float speed, 
               std::shared_ptr<Inventory> inv, entityid_t eid) :
    level(level),
    speed(speed),
    chosenSlot(0),
    position(position),
    inventory(std::move(inv)),
    eid(eid),
    camera(std::make_shared<Camera>(position, glm::radians(90.0f))),
    spCamera(std::make_shared<Camera>(position, glm::radians(90.0f))),
    tpCamera(std::make_shared<Camera>(position, glm::radians(90.0f))),
    currentCamera(camera)
{
}

Player::~Player() {
}

void Player::updateEntity() {
    if (eid == 0) {
        auto& def = level->content->entities.require("base:player");
        eid = level->entities->spawn(def, Transform {
            getPosition(), glm::vec3(1.0f), glm::mat3(1.0f), {}, true
        });
    } else if (auto entity = level->entities->get(eid)) {
        position = entity->getTransform().pos;
    } else {
        // check if chunk loaded
    }
}

Hitbox* Player::getHitbox() {
    if (auto entity = level->entities->get(eid)) {
        return &entity->getRigidbody().hitbox;
    }
    return nullptr;
}

void Player::updateInput(PlayerInput& input, float delta) {
    auto hitbox = getHitbox();
    if (hitbox == nullptr) {
        return;
    }
    bool crouch = input.shift && hitbox->grounded && !input.sprint;
    float speed = this->speed;
    if (flight){
        speed *= FLIGHT_SPEED_MUL;
    }
    if (input.cheat){
        speed *= CHEAT_SPEED_MUL;
    }

    if (crouch) {
        speed *= CROUCH_SPEED_MUL;
    } else if (input.sprint) {
        speed *= RUN_SPEED_MUL;
    }

    glm::vec3 dir(0,0,0);
    if (input.moveForward){
        dir += camera->dir;
    }
    if (input.moveBack){
        dir -= camera->dir;
    }
    if (input.moveRight){
        dir += camera->right;
    }
    if (input.moveLeft){
        dir -= camera->right;
    }
    if (glm::length(dir) > 0.0f){
        dir = glm::normalize(dir);
        hitbox->velocity += dir * speed * delta * 9.0f;
    }

    // physics calculation was here
                         
    if (flight && hitbox->grounded) {
        flight = false;
    }

    if (input.jump && hitbox->grounded){
        hitbox->velocity.y = JUMP_FORCE;
    }

    if ((input.flight && !noclip) ||
        (input.noclip && flight == noclip)){
        flight = !flight;
        if (flight){
            hitbox->grounded = false;
        }
    }
    if (input.noclip) {
        noclip = !noclip;
    }

    hitbox->linearDamping = PLAYER_GROUND_DAMPING;
    if (flight){
        hitbox->linearDamping = PLAYER_AIR_DAMPING;
        hitbox->velocity.y *= 1.0f - delta * 9;
        if (input.jump){
            hitbox->velocity.y += speed * delta * 9;
        }
        if (input.shift){
            hitbox->velocity.y -= speed * delta * 9;
        }
    }
    if (!hitbox->grounded) {
        hitbox->linearDamping = PLAYER_AIR_DAMPING;
    }

    input.noclip = false;
    input.flight = false;

    if (spawnpoint.y <= 0.1) {
        attemptToFindSpawnpoint();
    }
}

void Player::teleport(glm::vec3 position) {
    this->position = position;
    if (auto hitbox = getHitbox()) {
        hitbox->position = position;
    }
}

void Player::attemptToFindSpawnpoint() {
    glm::vec3 newpos (
        position.x + (rand() % 200 - 100),
        rand() % 80 + 100,
        position.z + (rand() % 200 - 100)
    );
    while (newpos.y > 0 && !level->chunks->isObstacleBlock(newpos.x, newpos.y-2, newpos.z)) {
        newpos.y--;
    }

    voxel* headvox = level->chunks->get(newpos.x, newpos.y+1, newpos.z);
    if (level->chunks->isObstacleBlock(newpos.x, newpos.y, newpos.z) ||
        headvox == nullptr || headvox->id != 0)
        return;
    spawnpoint = newpos + glm::vec3(0.5f, 0.0f, 0.5f);
    teleport(spawnpoint);
}

void Player::setChosenSlot(int index) {
    chosenSlot = index;
}

int Player::getChosenSlot() const {
    return chosenSlot;
}

float Player::getSpeed() const {
    return speed;
}

bool Player::isFlight() const {
    return flight;
}

void Player::setFlight(bool flag) {
    this->flight = flag;
}

bool Player::isNoclip() const {
    return noclip;
}

void Player::setNoclip(bool flag) {
    this->noclip = flag;
}

entityid_t Player::getEntity() const {
    return eid;
}

void Player::setEntity(entityid_t eid) {
    this->eid = eid;
}

std::shared_ptr<Inventory> Player::getInventory() const {
    return inventory;
}

void Player::setSpawnPoint(glm::vec3 spawnpoint) {
    this->spawnpoint = spawnpoint;
}

glm::vec3 Player::getSpawnPoint() const {
    return spawnpoint;
}

std::unique_ptr<dynamic::Map> Player::serialize() const {
    auto root = std::make_unique<dynamic::Map>();
    auto& posarr = root->putList("position");
    posarr.put(position.x);
    posarr.put(position.y);
    posarr.put(position.z);

    auto& rotarr = root->putList("rotation");
    rotarr.put(cam.x);
    rotarr.put(cam.y);
    rotarr.put(cam.z);

    auto& sparr = root->putList("spawnpoint");
    sparr.put(spawnpoint.x);
    sparr.put(spawnpoint.y);
    sparr.put(spawnpoint.z);

    root->put("flight", flight);
    root->put("noclip", noclip);
    root->put("chosen-slot", chosenSlot);
    root->put("entity", eid);
    root->put("inventory", inventory->serialize());
    return root;
}

void Player::deserialize(dynamic::Map *src) {
    auto posarr = src->list("position");
    position.x = posarr->num(0);
    position.y = posarr->num(1);
    position.z = posarr->num(2);
    camera->position = position;

    auto rotarr = src->list("rotation");
    cam.x = rotarr->num(0);
    cam.y = rotarr->num(1);
    if (rotarr->size() > 2) {
        cam.z = rotarr->num(2);
    }

    if (src->has("spawnpoint")) {
        auto sparr = src->list("spawnpoint");
        setSpawnPoint(glm::vec3(
            sparr->num(0),
            sparr->num(1),
            sparr->num(2)
        ));
    } else {
        setSpawnPoint(position);
    }

    src->flag("flight", flight);
    src->flag("noclip", noclip);
    setChosenSlot(src->get("chosen-slot", getChosenSlot()));
    src->num("entity", eid);
    
    if (auto invmap = src->map("inventory")) {
        getInventory()->deserialize(invmap.get());
    }
    std::cout << "Player::DESERIALIZE " << eid << std::endl;
}

void Player::convert(dynamic::Map* data, const ContentLUT* lut) {
    auto players = data->list("players");
    if (players) {
        for (uint i = 0; i < players->size(); i++) {
            auto playerData = players->map(i);
            if (auto inventory = playerData->map("inventory")) {
                Inventory::convert(inventory.get(), lut);
            }
        }
    
    } else {
        if (auto inventory = data->map("inventory")) {
            Inventory::convert(inventory.get(), lut);
        }
    }
}
