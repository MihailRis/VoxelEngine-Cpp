#include "Player.hpp"

#include "../content/ContentLUT.hpp"
#include "../physics/Hitbox.hpp"
#include "../physics/PhysicsSolver.hpp"
#include "../voxels/Chunks.hpp"
#include "../world/Level.hpp"
#include "../window/Events.hpp"
#include "../window/Camera.hpp"
#include "../items/Inventory.hpp"

#include <glm/glm.hpp>

const float CROUCH_SPEED_MUL = 0.35f;
const float RUN_SPEED_MUL = 1.5f;
const float PLAYER_GROUND_DAMPING = 10.0f;
const float PLAYER_AIR_DAMPING = 7.0f;
const float FLIGHT_SPEED_MUL = 4.0f;
const float CHEAT_SPEED_MUL = 5.0f;
const float JUMP_FORCE = 8.0f;

Player::Player(glm::vec3 position, float speed, std::shared_ptr<Inventory> inv) :
    speed(speed),
    chosenSlot(0),
    inventory(inv),
    camera(std::make_shared<Camera>(position, glm::radians(90.0f))),
    spCamera(std::make_shared<Camera>(position, glm::radians(90.0f))),
    tpCamera(std::make_shared<Camera>(position, glm::radians(90.0f))),
    currentCamera(camera),
    hitbox(std::make_unique<Hitbox>(position, glm::vec3(0.3f,0.9f,0.3f)))
{
}

Player::~Player() {
}

void Player::updateInput(
    Level* level,
    PlayerInput& input, 
    float delta
) {
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
        dir.x += camera->dir.x;
        dir.z += camera->dir.z;
    }
    if (input.moveBack){
        dir.x -= camera->dir.x;
        dir.z -= camera->dir.z;
    }
    if (input.moveRight){
        dir.x += camera->right.x;
        dir.z += camera->right.z;
    }
    if (input.moveLeft){
        dir.x -= camera->right.x;
        dir.z -= camera->right.z;
    }
    if (glm::length(dir) > 0.0f){
        dir = glm::normalize(dir);
        hitbox->velocity.x += dir.x * speed * delta * 9;
        hitbox->velocity.z += dir.z * speed * delta * 9;
    }

    float vel = glm::length(hitbox->velocity);
    int substeps = int(delta * vel * 20);
    substeps = std::min(100, std::max(2, substeps));
    level->physics->step(
        level->chunks.get(), 
        hitbox.get(), 
        delta, 
        substeps, 
        crouch, 
        flight ? 0.0f : 1.0f, 
        !noclip
    );
                         
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

    hitbox->linear_damping = PLAYER_GROUND_DAMPING;
    if (flight){
        hitbox->linear_damping = PLAYER_AIR_DAMPING;
        hitbox->velocity.y *= 1.0f - delta * 9;
        if (input.jump){
            hitbox->velocity.y += speed * delta * 9;
        }
        if (input.shift){
            hitbox->velocity.y -= speed * delta * 9;
        }
    }
    if (!hitbox->grounded) {
        hitbox->linear_damping = PLAYER_AIR_DAMPING;
    }

    input.noclip = false;
    input.flight = false;

    if (spawnpoint.y <= 0.1) {
        attemptToFindSpawnpoint(level);
    }
}

void Player::teleport(glm::vec3 position) {
    hitbox->position = position;
}

void Player::attemptToFindSpawnpoint(Level* level) {
    glm::vec3 ppos = hitbox->position;
    glm::vec3 newpos (
        ppos.x + (rand() % 200 - 100),
        rand() % 80 + 100,
        ppos.z + (rand() % 200 - 100)
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
    glm::vec3 position = hitbox->position;
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
    root->put("inventory", inventory->serialize());
    return root;
}

void Player::deserialize(dynamic::Map *src) {
    auto posarr = src->list("position");
    glm::vec3& position = hitbox->position;
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

    auto invmap = src->map("inventory");
    if (invmap) {
        getInventory()->deserialize(invmap);
    }
}


void Player::convert(dynamic::Map* data, const ContentLUT* lut) {
    auto players = data->list("players");
    if (players) {
        for (uint i = 0; i < players->size(); i++) {
            auto playerData = players->map(i);
            auto inventory = playerData->map("inventory");
            if (inventory) {
                Inventory::convert(inventory, lut);
            }
        }
    
    } else {
        auto inventory = data->map("inventory");
        if (inventory) {
            Inventory::convert(inventory, lut);
        }
    }
}
