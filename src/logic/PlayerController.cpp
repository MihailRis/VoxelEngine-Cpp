#define _USE_MATH_DEFINES
#include <cmath>

#include "PlayerController.hpp"
#include "BlocksController.hpp"
#include "scripting/scripting.hpp"

#include "../objects/Player.hpp"
#include "../objects/Entities.hpp"
#include "../physics/PhysicsSolver.hpp"
#include "../physics/Hitbox.hpp"
#include "../lighting/Lighting.hpp"
#include "../world/Level.hpp"
#include "../content/Content.hpp"
#include "../voxels/Block.hpp"
#include "../voxels/voxel.hpp"
#include "../voxels/Chunks.hpp"
#include "../window/Camera.hpp"
#include "../window/Window.hpp"
#include "../window/Events.hpp"
#include "../window/input.hpp"
#include "../items/ItemDef.hpp"
#include "../items/ItemStack.hpp"
#include "../items/Inventory.hpp"
#include "../core_defs.hpp"
#include "../settings.hpp"

#include <algorithm>

const float STEPS_SPEED = 2.2f;
const float CAM_SHAKE_OFFSET = 0.0075f;
const float CAM_SHAKE_OFFSET_Y = 0.031f;
const float CAM_SHAKE_SPEED = STEPS_SPEED;
const float CAM_SHAKE_DELTA_K = 10.0f;
const float ZOOM_SPEED = 16.0f;
const float CROUCH_ZOOM = 0.9f;
const float RUN_ZOOM = 1.1f;
const float C_ZOOM = 0.1f;
const float CROUCH_SHIFT_Y = -0.2f;

CameraControl::CameraControl(const std::shared_ptr<Player>& player, const CameraSettings& settings)
  : player(player), 
    camera(player->camera),
    settings(settings),
    offset(0.0f, 0.7f, 0.0f) {
}

void CameraControl::refresh() {
    camera->position = player->getPosition() + offset;
}

void CameraControl::updateMouse(PlayerInput& input) {
    glm::vec3& cam = player->cam;

    float sensitivity = (input.zoom 
        ? settings.sensitivity.get() / 4.f
        : settings.sensitivity.get());

    auto d = glm::degrees(Events::delta / (float)Window::height * sensitivity);
    cam.x -= d.x;
    cam.y -= d.y;
    
    if (cam.y < -89.9f) {
        cam.y = -89.9f;
    }
    else if (cam.y > 89.9f) {
        cam.y = 89.9f;
    }
    if (cam.x > 180.f) {
        cam.x -= 360.f;
    }
    else if (cam.x < -180.f) {
        cam.x += 360.f;
    }

    camera->rotation = glm::mat4(1.0f);
    camera->rotate(glm::radians(cam.y), glm::radians(cam.x), glm::radians(cam.z));
}

glm::vec3 CameraControl::updateCameraShaking(const Hitbox& hitbox, float delta) {
    glm::vec3 offset {};
    const float k = CAM_SHAKE_DELTA_K;
    const float ov = CAM_SHAKE_OFFSET_Y;
    const glm::vec3& vel = hitbox.velocity;

    interpVel = interpVel * (1.0f - delta * 5) + vel * delta * 0.1f;
    if (hitbox.grounded && interpVel.y < 0.0f){
        interpVel.y *= -30.0f;
    }
    shake = shake * (1.0f - delta * k);
    float oh = CAM_SHAKE_OFFSET;
    if (hitbox.grounded) {
        float f = glm::length(glm::vec2(vel.x, vel.z));
        shakeTimer += delta * f * CAM_SHAKE_SPEED;
        shake += f * delta * k;
        oh *= glm::sqrt(f);
    }
    
    offset += camera->right * glm::sin(shakeTimer) * oh * shake;
    offset += camera->up * glm::abs(glm::cos(shakeTimer)) * ov * shake;
    if (settings.inertia.get()) {
        offset -= glm::min(interpVel * 0.05f, 1.0f);
    }
    return offset;
}

void CameraControl::updateFovEffects(const Hitbox& hitbox, 
                                     PlayerInput input, float delta) {
    bool crouch = input.shift && hitbox.grounded && !input.sprint;

    float dt = fmin(1.0f, delta * ZOOM_SPEED);
    float zoomValue = 1.0f;
    if (crouch){
        offset += glm::vec3(0.f, CROUCH_SHIFT_Y, 0.f);
        zoomValue = CROUCH_ZOOM;
    } else if (input.sprint){
        zoomValue = RUN_ZOOM;
    }
    if (input.zoom)
        zoomValue *= C_ZOOM;
    camera->zoom = zoomValue * dt + camera->zoom * (1.0f - dt);
}

// temporary solution
// more extensible but uglier
void CameraControl::switchCamera() {
    const std::vector<std::shared_ptr<Camera>> playerCameras {
        camera, player->tpCamera, player->spCamera
    };

    auto index = std::distance(
        playerCameras.begin(),
        std::find_if(
            playerCameras.begin(), 
            playerCameras.end(), 
            [=](auto ptr) {
                return ptr.get() == player->currentCamera.get();
            }
        )
    );
    if (static_cast<size_t>(index) != playerCameras.size()) {
        index = (index + 1) % playerCameras.size();
        player->currentCamera = playerCameras.at(index);
    } else {
        player->currentCamera = camera;
    }
}

void CameraControl::update(PlayerInput input, float delta, Chunks* chunks) {
    offset = glm::vec3(0.0f, 0.0f, 0.0f);

    if (auto hitbox = player->getHitbox()) {
        offset.y += hitbox->halfsize.y * (0.7f/0.9f);
        if (settings.shaking.get() && !input.cheat) {
            offset += updateCameraShaking(*hitbox, delta);
        }
        if (settings.fovEffects.get()){
            updateFovEffects(*hitbox, input, delta);
        }
    }
    if (input.cameraMode) {
        switchCamera();
    }

    auto spCamera = player->spCamera;
    auto tpCamera = player->tpCamera;

    refresh();

    if (player->currentCamera == spCamera) {
        spCamera->position = chunks->rayCastToObstacle(
            camera->position, camera->front, 3.0f) - 0.4f * camera->front;
        spCamera->dir = -camera->dir;
        spCamera->front = -camera->front;
    }
    else if (player->currentCamera == tpCamera) {
        tpCamera->position = chunks->rayCastToObstacle(
            camera->position, -camera->front, 3.0f) + 0.4f * camera->front;
        tpCamera->dir = camera->dir;
        tpCamera->front = camera->front;
    }
    if (player->currentCamera == spCamera || 
        player->currentCamera == tpCamera ||
        player->currentCamera == camera) {
        player->currentCamera->setFov(glm::radians(settings.fov.get()));
    }
}

PlayerController::PlayerController(
    Level* level, 
    const EngineSettings& settings,
    BlocksController* blocksController
) : level(level), 
    player(level->getObject<Player>(0)), 
    camControl(player, settings.camera),
    blocksController(blocksController) 
{}


void PlayerController::onFootstep(const Hitbox& hitbox) {
    auto pos = hitbox.position;
    auto half = hitbox.halfsize;

    for (int offsetZ = -1; offsetZ <= 1; offsetZ++) {
        for (int offsetX = -1; offsetX <= 1; offsetX++) {
            int x = std::floor(pos.x+half.x*offsetX);
            int y = std::floor(pos.y-half.y*1.1f);
            int z = std::floor(pos.z+half.z*offsetZ);
            auto vox = level->chunks->get(x, y, z);
            if (vox) {
                auto def = level->content->getIndices()->blocks.get(vox->id);
                if (!def->obstacle)
                    continue;
                blocksController->onBlockInteraction(
                    player.get(),
                    glm::ivec3(x, y, z), def,
                    BlockInteraction::step
                );
                return;
            }
        }
    }
}

void PlayerController::updateFootsteps(float delta) {
    auto hitbox = player->getHitbox();
    if (hitbox && hitbox->grounded) {
        const glm::vec3& vel = hitbox->velocity;
        float f = glm::length(glm::vec2(vel.x, vel.z));
        stepsTimer += delta * f * STEPS_SPEED;
        if (stepsTimer >= M_PI) {
            stepsTimer = fmod(stepsTimer, M_PI);
            onFootstep(*hitbox);
        }
    } else {
        stepsTimer = M_PI;
    }
}

void PlayerController::update(float delta, bool input, bool pause) {
    if (!pause) {
        if (input) {
            updateKeyboard();
            player->updateSelectedEntity();
        } else {
            resetKeyboard();
        }
        updatePlayer(delta);
    }
}

void PlayerController::postUpdate(float delta, bool input, bool pause) {
    if (!pause) {
        updateFootsteps(delta);
    }

    if (!pause && input) {
        camControl.updateMouse(this->input);
    }
    if (input) {
        updateInteraction();
    } else {
        player->selection = {};
    }
    player->postUpdate();
    camControl.update(this->input, delta, level->chunks.get());
}

void PlayerController::updateKeyboard() {
    input.moveForward = Events::active(BIND_MOVE_FORWARD);
    input.moveBack = Events::active(BIND_MOVE_BACK);
    input.moveLeft = Events::active(BIND_MOVE_LEFT);
    input.moveRight = Events::active(BIND_MOVE_RIGHT);
    input.sprint = Events::active(BIND_MOVE_SPRINT);
    input.shift = Events::active(BIND_MOVE_CROUCH);
    input.cheat = Events::active(BIND_MOVE_CHEAT);
    input.jump = Events::active(BIND_MOVE_JUMP);
    input.zoom = Events::active(BIND_CAM_ZOOM);
    input.cameraMode = Events::jactive(BIND_CAM_MODE);
    input.noclip = Events::jactive(BIND_PLAYER_NOCLIP);
    input.flight = Events::jactive(BIND_PLAYER_FLIGHT);
}

void PlayerController::resetKeyboard() {
    input.zoom = false;
    input.moveForward = false;
    input.moveBack = false;
    input.moveLeft = false;
    input.moveRight = false;
    input.sprint = false;
    input.shift = false;
    input.cheat = false;
    input.jump = false;
}

void PlayerController::updatePlayer(float delta) {
    player->updateEntity();
    player->updateInput(input, delta);
}

static int determine_rotation(Block* def, const glm::ivec3& norm, glm::vec3& camDir) {
    if (def && def->rotatable){
        const std::string& name = def->rotations.name;
        if (name == "pipe") {
            if (norm.x < 0.0f) return BLOCK_DIR_WEST;
            else if (norm.x > 0.0f) return BLOCK_DIR_EAST;
            else if (norm.y > 0.0f) return BLOCK_DIR_UP;
            else if (norm.y < 0.0f) return BLOCK_DIR_DOWN;
            else if (norm.z > 0.0f) return BLOCK_DIR_NORTH;
            else if (norm.z < 0.0f) return BLOCK_DIR_SOUTH;
        } 
        else if (name == "pane") {
            if (abs(camDir.x) > abs(camDir.z)){
                if (camDir.x > 0.0f) return BLOCK_DIR_EAST;
                if (camDir.x < 0.0f) return BLOCK_DIR_WEST;
            }
            if (abs(camDir.x) < abs(camDir.z)){
                if (camDir.z > 0.0f) return BLOCK_DIR_SOUTH;
                if (camDir.z < 0.0f) return BLOCK_DIR_NORTH;
            }
        }
    }
    return 0;
}

static void pick_block(ContentIndices* indices, Chunks* chunks, Player* player,
                       int x, int y, int z) {
    auto block = indices->blocks.get(chunks->get(x,y,z)->id);
    itemid_t id = block->rt.pickingItem;
    auto inventory = player->getInventory();
    size_t slotid = inventory->findSlotByItem(id, 0, 10);
    if (slotid == Inventory::npos) {
        slotid = player->getChosenSlot();
    } else {
        player->setChosenSlot(slotid);
    }
    ItemStack& stack = inventory->getSlot(slotid);
    if (stack.getItemId() != id) {
        stack.set(ItemStack(id, 1));
    }
}

voxel* PlayerController::updateSelection(float maxDistance) {
    auto indices = level->content->getIndices();
    auto chunks = level->chunks.get();
    auto camera = player->camera.get();
    auto& selection = player->selection;

    glm::vec3 end;
    glm::ivec3 iend;
    glm::ivec3 norm;
    voxel* vox = chunks->rayCast(
        camera->position, 
        camera->front, 
        maxDistance, 
        end, norm, iend
    );
    if (vox) {
        maxDistance = glm::distance(camera->position, end);
    }
    auto prevEntity = selection.entity;
    selection.entity = ENTITY_NONE;
    selection.actualPosition = iend;
    if (auto result = level->entities->rayCast(
        camera->position, camera->front, maxDistance, player->getEntity())) {
        selection.entity = result->entity;
        selection.hitPosition = camera->position + camera->front * result->distance;
        selection.position = selection.hitPosition;
        selection.actualPosition = selection.position;
        selection.normal = result->normal;
    }
    if (selection.entity != prevEntity) {
        if (prevEntity != ENTITY_NONE) {
            if (auto pentity = level->entities->get(prevEntity)) {
                scripting::on_aim_off(*pentity, player.get());
            }
        }
        if (selection.entity != ENTITY_NONE) {
            if (auto pentity = level->entities->get(selection.entity)) {
                scripting::on_aim_on(*pentity, player.get());
            }
        }
    }
    if (vox == nullptr || selection.entity) {
        selection.vox = {BLOCK_VOID, {}};
        return nullptr;
    }
    blockstate selectedState = vox->state;
    selection.vox = *vox;
    if (selectedState.segment) {
        selection.position = chunks->seekOrigin(
            iend, indices->blocks.get(selection.vox.id), selectedState
        );
        auto origin = chunks->get(selection.position);
        if (origin && origin->id != vox->id) {
            chunks->set(iend.x, iend.y, iend.z, 0, {});
            return updateSelection(maxDistance);
        }
    } else {
        selection.position = iend;
    }
    selection.hitPosition = end;
    selection.normal = norm;
    return vox;
}

void PlayerController::processRightClick(Block* def, Block* target) {
    const auto& selection = player->selection;
    auto chunks = level->chunks.get();
    auto camera = player->camera.get();

    blockstate state {};
    state.rotation = determine_rotation(def, selection.normal, camera->dir);

    if (!input.shift && target->rt.funcsset.oninteract) {
        if (scripting::on_block_interact(player.get(), target, selection.position)) {
            return;
        }
    }
    auto coord = selection.actualPosition;
    if (!target->replaceable){
        coord += selection.normal;
    } else if (def->rotations.name == BlockRotProfile::PIPE_NAME) {
        state.rotation = BLOCK_DIR_UP;
    }
    blockid_t chosenBlock = def->rt.id;

    AABB blockAABB(coord, coord+1);
    bool blocked = level->entities->hasBlockingInside(blockAABB);

    if (def->obstacle && blocked) {
        return;
    }
    auto vox = chunks->get(coord);
    if (vox == nullptr) {
        return;
    }
    if (!chunks->checkReplaceability(def, state, coord)) {
        return;
    }
    if (def->grounded) {
        const auto& vec = get_ground_direction(def, state.rotation);
        if (!chunks->isSolidBlock(coord.x+vec.x, coord.y+vec.y, coord.z+vec.z)) {
            return;
        }
    }
    if (chosenBlock != vox->id && chosenBlock) {
        blocksController->placeBlock(
            player.get(), def, state, coord.x, coord.y, coord.z);
    }
}

void PlayerController::updateEntityInteraction(entityid_t eid, bool lclick, bool rclick) {
    auto entityOpt = level->entities->get(eid);
    if (!entityOpt.has_value()) {
        return;
    }
    auto entity = entityOpt.value();
    if (lclick) {
        scripting::on_attacked(entity, player.get(), player->getEntity());
    }
    if (rclick) {
        scripting::on_entity_used(entity, player.get());
    }
}

void PlayerController::updateInteraction() {
    auto indices = level->content->getIndices();
    auto chunks = level->chunks.get();
    const auto& selection = player->selection;

    bool xkey = Events::pressed(keycode::X);
    bool lclick = Events::jactive(BIND_PLAYER_ATTACK) || (xkey && Events::active(BIND_PLAYER_ATTACK));
    bool rclick = Events::jactive(BIND_PLAYER_BUILD) || (xkey && Events::active(BIND_PLAYER_BUILD));
    float maxDistance = xkey ? 200.0f : 10.0f;
    
    auto inventory = player->getInventory();
    const ItemStack& stack = inventory->getSlot(player->getChosenSlot());
    ItemDef* item = indices->items.get(stack.getItemId());

    auto vox = updateSelection(maxDistance);
    if (vox == nullptr) {
        if (rclick && item->rt.funcsset.on_use) {
            scripting::on_item_use(player.get(), item);
        }
        if (selection.entity) {
            updateEntityInteraction(selection.entity, lclick, rclick);
        }
        return;
    }
    
    auto iend = selection.position;
    if (lclick && !input.shift && item->rt.funcsset.on_block_break_by) {
        if (scripting::on_item_break_block(player.get(), item, iend.x, iend.y, iend.z)) {
            return;
        }
    }
    auto target = indices->blocks.get(vox->id);
    if (lclick && target->breakable){
        blocksController->breakBlock(player.get(), target, iend.x, iend.y, iend.z);
    }
    if (rclick && !input.shift) {
        bool preventDefault = false;
        if (item->rt.funcsset.on_use_on_block) {
            preventDefault = scripting::on_item_use_on_block(
                player.get(), item, iend, selection.normal
            );
        } else if (item->rt.funcsset.on_use) {
            preventDefault = scripting::on_item_use(player.get(), item);
        }
        if (preventDefault) {
            return;
        }
    }
    auto def = indices->blocks.get(item->rt.placingBlock);
    if (def && rclick) {
        processRightClick(def, target);
    }
    if (Events::jactive(BIND_PLAYER_PICK)) {
        auto coord = selection.actualPosition;
        pick_block(indices, chunks, player.get(), coord.x, coord.y, coord.z);
    }
}

Player* PlayerController::getPlayer() {
    return player.get();
}
