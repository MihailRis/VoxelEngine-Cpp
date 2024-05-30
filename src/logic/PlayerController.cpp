#define _USE_MATH_DEFINES
#include <cmath>

#include "PlayerController.hpp"
#include "BlocksController.hpp"
#include "scripting/scripting.hpp"

#include "../objects/Player.hpp"
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

const float CAM_SHAKE_OFFSET = 0.025f;
const float CAM_SHAKE_OFFSET_Y = 0.031f;
const float CAM_SHAKE_SPEED = 1.75f;
const float CAM_SHAKE_DELTA_K = 10.0f;
const float STEPS_SPEED = 1.75f;
const float ZOOM_SPEED = 16.0f;
const float CROUCH_ZOOM = 0.9f;
const float RUN_ZOOM = 1.1f;
const float C_ZOOM = 0.1f;
const float CROUCH_SHIFT_Y = -0.2f;

CameraControl::CameraControl(std::shared_ptr<Player> player, const CameraSettings& settings) 
  : player(player), 
    camera(player->camera),
    settings(settings),
    offset(0.0f, 0.7f, 0.0f) {
}

void CameraControl::refresh() {
    camera->position = player->hitbox->position + offset;
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

glm::vec3 CameraControl::updateCameraShaking(float delta) {
    glm::vec3 offset {};
    auto hitbox = player->hitbox.get();
    const float k = CAM_SHAKE_DELTA_K;
    const float oh = CAM_SHAKE_OFFSET;
    const float ov = CAM_SHAKE_OFFSET_Y;
    const glm::vec3& vel = hitbox->velocity;

    interpVel = interpVel * (1.0f - delta * 5) + vel * delta * 0.1f;
    if (hitbox->grounded && interpVel.y < 0.0f){
        interpVel.y *= -30.0f;
    }
    shake = shake * (1.0f - delta * k);
    if (hitbox->grounded) {
        float f = glm::length(glm::vec2(vel.x, vel.z));
        shakeTimer += delta * f * CAM_SHAKE_SPEED;
        shake += f * delta * k;
    }
    offset += camera->right * glm::sin(shakeTimer) * oh * shake;
    offset += camera->up * glm::abs(glm::cos(shakeTimer)) * ov * shake;
    offset -= glm::min(interpVel * 0.05f, 1.0f);
    return offset;
}

void CameraControl::updateFovEffects(const PlayerInput& input, float delta) {
    auto hitbox = player->hitbox.get();
    bool crouch = input.shift && hitbox->grounded && !input.sprint;

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
    }
}

void CameraControl::update(const PlayerInput& input, float delta, Chunks* chunks) {
    offset = glm::vec3(0.0f, 0.7f, 0.0f);

    if (settings.shaking.get() && !input.cheat) {
        offset += updateCameraShaking(delta);
    }
    if (settings.fovEffects.get()){
        updateFovEffects(input, delta);
    }
    if (input.cameraMode) {
        switchCamera();
    }

    auto spCamera = player->spCamera;
    auto tpCamera = player->tpCamera;

    if (player->currentCamera == spCamera) {
        spCamera->position = chunks->rayCastToObstacle(camera->position, camera->front, 3.0f) - 0.2f * camera->front;
        spCamera->dir = -camera->dir;
        spCamera->front = -camera->front;
    }
    else if (player->currentCamera == tpCamera) {
        tpCamera->position = chunks->rayCastToObstacle(camera->position, -camera->front, 3.0f) + 0.2f * camera->front;
        tpCamera->dir = camera->dir;
        tpCamera->front = camera->front;
    }
}

glm::vec3 PlayerController::selectedBlockPosition;
glm::vec3 PlayerController::selectedPointPosition;
glm::ivec3 PlayerController::selectedBlockNormal;
int PlayerController::selectedBlockId = -1;
int PlayerController::selectedBlockRotation = 0;

PlayerController::PlayerController(
    Level* level, 
    const EngineSettings& settings,
    BlocksController* blocksController
) : level(level), 
    player(level->getObject<Player>(0)), 
    camControl(player, settings.camera),
    blocksController(blocksController) 
{}

void PlayerController::onBlockInteraction(
    glm::ivec3 pos,
    const Block* def,
    BlockInteraction type
) {
    for (auto callback : blockInteractionCallbacks) {
        callback(player.get(), pos, def, type);
    }
}

void PlayerController::onFootstep() {
    auto hitbox = player->hitbox.get();
    glm::vec3 pos = hitbox->position;
    glm::vec3 half = hitbox->halfsize;

    for (int offsetZ = -1; offsetZ <= 1; offsetZ++) {
        for (int offsetX = -1; offsetX <= 1; offsetX++) {
            int x = std::floor(pos.x+half.x*offsetX);
            int y = std::floor(pos.y-half.y*1.1f);
            int z = std::floor(pos.z+half.z*offsetZ);
            auto vox = level->chunks->get(x, y, z);
            if (vox) {
                auto def = level->content->getIndices()->getBlockDef(vox->id);
                if (!def->obstacle)
                    continue;
                onBlockInteraction(
                    glm::ivec3(x, y, z), def,
                    BlockInteraction::step
                );
                return;
            }
        }
    }
}

void PlayerController::updateFootsteps(float delta) {
    auto hitbox = player->hitbox.get();

    if (hitbox->grounded) {
        const glm::vec3& vel = hitbox->velocity;
        float f = glm::length(glm::vec2(vel.x, vel.z));
        stepsTimer += delta * f * STEPS_SPEED;
        if (stepsTimer >= M_PI) {
            stepsTimer = fmod(stepsTimer, M_PI);
            onFootstep();
        }
    } else {
        stepsTimer = M_PI;
    }
}

void PlayerController::update(float delta, bool input, bool pause) {
    if (!pause) {
        if (input) {
            updateKeyboard();
        } else {
            resetKeyboard();
        }
        updateFootsteps(delta);
        updateCamera(delta, input);
        updateControls(delta);

    }
    camControl.refresh();
    if (input) {
        updateInteraction();
    } else {
        selectedBlockId = -1;
        selectedBlockRotation = 0;
    }
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

void PlayerController::updateCamera(float delta, bool movement) {
    if (movement) {
        camControl.updateMouse(input);
    }
    camControl.update(input, delta, level->chunks.get());
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

void PlayerController::updateControls(float delta){
    player->updateInput(level, input, delta);
}

static int determine_rotation(Block* def, glm::ivec3& norm, glm::vec3& camDir) {
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

static void pick_block(ContentIndices* indices, Chunks* chunks, Player* player, int x, int y, int z) {
    Block* block = indices->getBlockDef(chunks->get(x,y,z)->id);
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

// TODO: refactor this nesting nest
void PlayerController::updateInteraction(){
    auto indices = level->content->getIndices();
    Chunks* chunks = level->chunks.get();
    Lighting* lighting = level->lighting.get();
    Camera* camera = player->camera.get();

    bool xkey = Events::pressed(keycode::X);
    bool lclick = Events::jactive(BIND_PLAYER_ATTACK) || 
                  (xkey && Events::active(BIND_PLAYER_ATTACK));
    bool rclick = Events::jactive(BIND_PLAYER_BUILD) || 
                  (xkey && Events::active(BIND_PLAYER_BUILD));
    float maxDistance = 10.0f;
    if (xkey) {
        maxDistance *= 20.0f;
    }
    auto inventory = player->getInventory();
    const ItemStack& stack = inventory->getSlot(player->getChosenSlot());
    ItemDef* item = indices->getItemDef(stack.getItemId());

    glm::vec3 end;
    glm::ivec3 iend;
    glm::ivec3 norm;
    voxel* vox = chunks->rayCast(
        camera->position, 
        camera->front, 
        maxDistance, 
        end, norm, iend
    );
    if (vox != nullptr){
        player->selectedVoxel = *vox;
        selectedBlockId = vox->id;
        selectedBlockRotation = vox->rotation();
        selectedBlockPosition = iend;
        selectedPointPosition = end;
        selectedBlockNormal = norm;
        int x = iend.x;
        int y = iend.y;
        int z = iend.z;

        Block* def = indices->getBlockDef(item->rt.placingBlock);
        uint8_t states = determine_rotation(def, norm, camera->dir);
        
        if (lclick && !input.shift && item->rt.funcsset.on_block_break_by) {
            if (scripting::on_item_break_block(player.get(), item, x, y, z))
                return;
        }

        Block* target = indices->getBlockDef(vox->id);
        if (lclick && target->breakable){
            onBlockInteraction(
                glm::ivec3(x, y, z), target,
                BlockInteraction::destruction
            );
            blocksController->breakBlock(player.get(), target, x, y, z);
        }
        if (rclick && !input.shift) {
            bool preventDefault = false;
            if (item->rt.funcsset.on_use_on_block) {
                preventDefault = scripting::on_item_use_on_block(player.get(), item, x, y, z);
            } else if (item->rt.funcsset.on_use) {
                preventDefault = scripting::on_item_use(player.get(), item);
            }
            if (preventDefault) {
                return;
            }
        }
        if (def && rclick){
            if (!input.shift && target->rt.funcsset.oninteract) {
                if (scripting::on_block_interact(player.get(), target, x, y, z))
                    return;
            }
            if (!target->replaceable){
                x = (iend.x)+(norm.x);
                y = (iend.y)+(norm.y);
                z = (iend.z)+(norm.z);
            } else {
                if (def->rotations.name == "pipe") {
                    states = BLOCK_DIR_UP;
                }
            }
            vox = chunks->get(x, y, z);
            blockid_t chosenBlock = def->rt.id;
            if (vox && (target = indices->getBlockDef(vox->id))->replaceable) {
                if (!level->physics->isBlockInside(x,y,z,def,states, player->hitbox.get()) 
                    || !def->obstacle){
                    if (def->grounded && !chunks->isSolidBlock(x, y-1, z)) {
                        chosenBlock = 0;
                    }
                    if (chosenBlock != vox->id && chosenBlock) {
                        onBlockInteraction(
                            glm::ivec3(x, y, z), def,
                            BlockInteraction::placing
                        );
                        chunks->set(x, y, z, chosenBlock, states);
                        lighting->onBlockSet(x,y,z, chosenBlock);
                        if (def->rt.funcsset.onplaced) {
                            scripting::on_block_placed(player.get(), def, x, y, z);
                        }
                        blocksController->updateSides(x, y, z);
                    }
                }
            }
        }
        if (Events::jactive(BIND_PLAYER_PICK)) {
            pick_block(indices, chunks, player.get(), x, y, z);
        }
    } else {
        selectedBlockId = -1;
        selectedBlockRotation = 0;
    }
    if (rclick) {
        if (item->rt.funcsset.on_use) {
            scripting::on_item_use(player.get(), item);
        } 
    }
}

Player* PlayerController::getPlayer() {
    return player.get();
}

void PlayerController::listenBlockInteraction(on_block_interaction callback) {
    blockInteractionCallbacks.push_back(callback);
}
