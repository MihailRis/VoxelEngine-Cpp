#include "PlayerController.h"

#include "../objects/Player.h"
#include "../physics/PhysicsSolver.h"
#include "../physics/Hitbox.h"
#include "../lighting/Lighting.h"
#include "../world/Level.h"
#include "../content/Content.h"
#include "../voxels/Block.h"
#include "../voxels/voxel.h"
#include "../voxels/Chunks.h"
#include "../window/Camera.h"
#include "../window/Events.h"
#include "../window/input.h"
#include "../items/ItemDef.h"
#include "../items/ItemStack.h"
#include "../items/Inventory.h"
#include "scripting/scripting.h"
#include "BlocksController.h"

#include "../core_defs.h"

const float CAM_SHAKE_OFFSET = 0.025f;
const float CAM_SHAKE_OFFSET_Y = 0.031f;
const float CAM_SHAKE_SPEED = 1.75f;
const float CAM_SHAKE_DELTA_K = 10.0f;
const float ZOOM_SPEED = 16.0f;
const float CROUCH_ZOOM = 0.9f;
const float RUN_ZOOM = 1.1f;
const float C_ZOOM = 0.1f;
const float CROUCH_SHIFT_Y = -0.2f;


CameraControl::CameraControl(Player* player, const CameraSettings& settings) 
	: player(player), 
	  camera(player->camera), 
	  currentViewCamera(player->currentCamera),
	  settings(settings),
	  offset(0.0f, 0.7f, 0.0f) {
}

void CameraControl::refresh() {
	camera->position = player->hitbox->position + offset;
}

void CameraControl::updateMouse(PlayerInput& input) {
	glm::vec2& cam = player->cam;

	float sensitivity = (input.zoom ? settings.sensitivity / 4.f : settings.sensitivity);
	cam -= glm::degrees(Events::delta / (float)Window::height * sensitivity);

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
	camera->rotate(glm::radians(cam.y), glm::radians(cam.x), 0);
}

void CameraControl::update(PlayerInput& input, float delta, Chunks* chunks) {
	Hitbox* hitbox = player->hitbox.get();

	offset = glm::vec3(0.0f, 0.7f, 0.0f);

	if (settings.shaking && !input.cheat) {
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
	}

	if (settings.fovEvents){
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

    auto spCamera = player->spCamera;
    auto tpCamera = player->tpCamera;

	if (input.cameraMode) { //ugly but effective
		if (player->currentCamera == camera)
			player->currentCamera = tpCamera;
		else if (player->currentCamera == spCamera)
			player->currentCamera = camera;
		else if (player->currentCamera == tpCamera)
			player->currentCamera = spCamera;
	}
	if (player->currentCamera == spCamera) {
		spCamera->position = chunks->rayCastToObstacle(camera->position, camera->front, 3.0f) - 0.2f*(camera->front);
		spCamera->dir = -camera->dir;
		spCamera->front = -camera->front;
	}
	else if (player->currentCamera == tpCamera) {
		tpCamera->position = chunks->rayCastToObstacle(camera->position, -camera->front, 3.0f) + 0.2f * (camera->front);
		tpCamera->dir = camera->dir;
		tpCamera->front = camera->front;
	}
}

glm::vec3 PlayerController::selectedBlockPosition;
glm::vec3 PlayerController::selectedPointPosition;
glm::ivec3 PlayerController::selectedBlockNormal;
int PlayerController::selectedBlockId = -1;
int PlayerController::selectedBlockStates = 0;

PlayerController::PlayerController(
    Level* level, 
    const EngineSettings& settings,
    BlocksController* blocksController) 
	: level(level), 
	  player(level->player), 
	  camControl(level->player, settings.camera),
      blocksController(blocksController) {
}

void PlayerController::update(float delta, bool input, bool pause) {
	if (!pause) {
		if (input) {
			updateKeyboard();
		} else {
			resetKeyboard();
		}
        updateCamera(delta, input);
		updateControls(delta);

	}
	camControl.refresh();
	if (input) {
		updateInteraction();
	} else {
		selectedBlockId = -1;
		selectedBlockStates = 0;
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
	camControl.update(input, delta, level->chunks);
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
	player->update(level, input, delta);
}

void PlayerController::updateInteraction(){
	auto indices = level->content->getIndices();
	Chunks* chunks = level->chunks;
	Player* player = level->player;
	Lighting* lighting = level->lighting;
	Camera* camera = player->camera.get();
	glm::vec3 end;
	glm::ivec3 iend;
	glm::ivec3 norm;

	bool xkey = Events::pressed(keycode::X);
	bool lclick = Events::jactive(BIND_PLAYER_ATTACK) || 
				  (xkey && Events::active(BIND_PLAYER_ATTACK));
	bool rclick = Events::jactive(BIND_PLAYER_BUILD) || 
				  (xkey && Events::active(BIND_PLAYER_BUILD));
	float maxDistance = 10.0f;
	if (xkey) {
		maxDistance *= 20.0f;
	}

	voxel* vox = chunks->rayCast(camera->position, 
								 camera->front, 
								 maxDistance, 
								 end, norm, iend);
	if (vox != nullptr){
		player->selectedVoxel = *vox;
		selectedBlockId = vox->id;
		selectedBlockStates = vox->states;
		selectedBlockPosition = iend;
		selectedPointPosition = end;
		selectedBlockNormal = norm;
		int x = iend.x;
		int y = iend.y;
		int z = iend.z;
		uint8_t states = 0;

        auto inventory = player->getInventory();
        ItemStack& stack = inventory->getSlot(player->getChosenSlot());
        ItemDef* item = indices->getItemDef(stack.getItemId());
		Block* def = indices->getBlockDef(item->rt.placingBlock);
		if (def && def->rotatable){
			const std::string& name = def->rotations.name;
			if (name == "pipe") {
				if (norm.x < 0.0f) states = BLOCK_DIR_WEST;
				else if (norm.x > 0.0f) states = BLOCK_DIR_EAST;
				else if (norm.y > 0.0f) states = BLOCK_DIR_UP;
				else if (norm.y < 0.0f) states = BLOCK_DIR_DOWN;
				else if (norm.z > 0.0f) states = BLOCK_DIR_NORTH;
				else if (norm.z < 0.0f) states = BLOCK_DIR_SOUTH;
			} else if (name == "pane") {
				glm::vec3 vec = camera->dir;
				if (abs(vec.x) > abs(vec.z)){
					if (vec.x > 0.0f) states = BLOCK_DIR_EAST;
					if (vec.x < 0.0f) states = BLOCK_DIR_WEST;
				}
				if (abs(vec.x) < abs(vec.z)){
					if (vec.z > 0.0f) states = BLOCK_DIR_SOUTH;
					if (vec.z < 0.0f) states = BLOCK_DIR_NORTH;
				}
			}
		}
		
        if (lclick) {
            if (!input.shift && item->rt.funcsset.on_block_break_by) {
                if (scripting::on_item_break_block(player, item, x, y, z))
                    return;
            } 
        }

		Block* target = indices->getBlockDef(vox->id);
		if (lclick && target->breakable){
            blocksController->breakBlock(player, target, x, y, z);
		}
        if (rclick) {
            if (!input.shift && item->rt.funcsset.on_use_on_block) {
                if (scripting::on_item_use_on_block(player, item, x, y, z))
                    return;
            } 
        }
		if (def && rclick){
            if (!input.shift && target->rt.funcsset.oninteract) {
                scripting::on_block_interact(player, target, x, y, z);
                return;
            }
			if (!target->replaceable){
				x = (iend.x)+(norm.x);
				y = (iend.y)+(norm.y);
				z = (iend.z)+(norm.z);
			}
			vox = chunks->get(x, y, z);
            blockid_t chosenBlock = def->rt.id;
			if (vox && (target = indices->getBlockDef(vox->id))->replaceable) {
				if (!level->physics->isBlockInside(x,y,z, player->hitbox.get()) 
					|| !def->obstacle){
                    if (def->grounded && !chunks->isSolidBlock(x, y-1, z)) {
                        chosenBlock = 0;
                    }
                    if (chosenBlock != vox->id) {
                        chunks->set(x, y, z, chosenBlock, states);
                        lighting->onBlockSet(x,y,z, chosenBlock);
                        if (def->rt.funcsset.onplaced) {
                            scripting::on_block_placed(player, def, x, y, z);
                        }
                        blocksController->updateSides(x, y, z);
                    }
				}
			}
		}
		if (Events::jactive(BIND_PLAYER_PICK)){
            Block* block = indices->getBlockDef(chunks->get(x,y,z)->id);
			itemid_t id = block->rt.pickingItem;
			auto inventory = player->getInventory();
			size_t slotid = inventory->findSlotByItem(id);
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
	} else {
		selectedBlockId = -1;
		selectedBlockStates = 0;
	}
}
