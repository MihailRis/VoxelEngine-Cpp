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

#include "../core_defs.h"

const float CAM_SHAKE_OFFSET = 0.025f;
const float CAM_SHAKE_OFFSET_Y = 0.031f;
const float CAM_SHAKE_SPEED = 1.6f;
const float CAM_SHAKE_DELTA_K = 10.0f;
const float ZOOM_SPEED = 16.0f;
const float CROUCH_ZOOM = 0.9f;
const float RUN_ZOOM = 1.1f;
const float C_ZOOM = 0.1f;
const float CROUCH_SHIFT_Y = -0.2f;

using glm::vec2;
using glm::vec3;
using std::string;

CameraControl::CameraControl(Player* player, const CameraSettings& settings) 
	: player(player), 
	  camera(player->camera), 
	  settings(settings),
	  offset(0.0f, 0.7f, 0.0f) {
}

void CameraControl::refresh() {
	camera->position = player->hitbox->position + offset;
}

void CameraControl::updateMouse(PlayerInput& input) {
	float rotX = -Events::deltaX / Window::height * 2;
	float rotY = -Events::deltaY / Window::height * 2;

	if (input.zoom){
		rotX /= 4;
		rotY /= 4;
	}

	float& camX = player->camX;
	float& camY = player->camY;
	camX += rotX;
	camY += rotY;
	if (camY < -radians(89.9f)){
		camY = -radians(89.9f);
	}
	if (camY > radians(89.9f)){
		camY = radians(89.9f);
	}

	camera->rotation = mat4(1.0f);
	camera->rotate(camY, camX, 0);
}

void CameraControl::update(PlayerInput& input, float delta) {
	Hitbox* hitbox = player->hitbox;

	offset = vec3(0.0f, 0.7f, 0.0f);

	if (settings.shaking && !input.cheat) {
		const float k = CAM_SHAKE_DELTA_K;
		const float oh = CAM_SHAKE_OFFSET;
		const float ov = CAM_SHAKE_OFFSET_Y;
		const vec3& vel = hitbox->velocity;

		interpVel = interpVel * (1.0f - delta * 5) + vel * delta * 0.1f;
		if (hitbox->grounded && interpVel.y < 0.0f){
			interpVel.y *= -30.0f;
		}
		shake = shake * (1.0f - delta * k);
		if (hitbox->grounded) {
			float f = length(vec2(vel.x, vel.z));
			shakeTimer += delta * f * CAM_SHAKE_SPEED;
			shake += f * delta * k;
		}
		offset += camera->right * sin(shakeTimer) * oh * shake;
		offset += camera->up * abs(cos(shakeTimer)) * ov * shake;
		offset -= glm::min(interpVel * 0.05f, 1.0f);
	}

	if (settings.fovEvents){
		bool crouch = input.shift && hitbox->grounded && !input.sprint;

		float dt = fmin(1.0f, delta * ZOOM_SPEED);
		float zoomValue = 1.0f;
		if (crouch){
			offset += vec3(0.f, CROUCH_SHIFT_Y, 0.f);
			zoomValue = CROUCH_ZOOM;
		} else if (input.sprint){
			zoomValue = RUN_ZOOM;
		}
		if (input.zoom)
			zoomValue *= C_ZOOM;
		camera->zoom = zoomValue * dt + camera->zoom * (1.0f - dt);
	}
}

vec3 PlayerController::selectedBlockPosition;
vec3 PlayerController::selectedPointPosition;
vec3 PlayerController::selectedBlockNormal;
int PlayerController::selectedBlockId = -1;
int PlayerController::selectedBlockStates = 0;

PlayerController::PlayerController(Level* level, const EngineSettings& settings) 
	: level(level), 
	  player(level->player), 
	  camControl(level->player, settings.camera) {
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

	input.noclip = Events::jactive(BIND_PLAYER_NOCLIP);
	input.flight = Events::jactive(BIND_PLAYER_FLIGHT);

	// block choice
	for (int i = 1; i < 10; i++){
		if (Events::jpressed(keycode::NUM_0+i)){
			player->choosenBlock = i;
		}
	}
}

void PlayerController::updateCamera(float delta, bool movement) {
	if (movement) {
		camControl.updateMouse(input);
	}
	camControl.update(input, delta);
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
	auto contentIds = level->content->indices;
	Chunks* chunks = level->chunks;
	Player* player = level->player;
	Lighting* lighting = level->lighting;
	Camera* camera = player->camera;
	vec3 end;
	vec3 norm;

	bool xkey = Events::pressed(keycode::X);
	bool lclick = Events::jactive(BIND_PLAYER_ATTACK) || 
				  (xkey && Events::active(BIND_PLAYER_ATTACK));
	bool rclick = Events::jactive(BIND_PLAYER_BUILD) || 
				  (xkey && Events::active(BIND_PLAYER_BUILD));
	float maxDistance = 10.0f;
	if (xkey) {
		maxDistance *= 20.0f;
	}
	vec3 iend;
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
		int x = (int)iend.x;
		int y = (int)iend.y;
		int z = (int)iend.z;
		uint8_t states = 0;

		Block* def = contentIds->getBlockDef(player->choosenBlock);
		if (def->rotatable){
			const string& name = def->rotations.name;
			if (name == "pipe") {
				if (norm.x < 0.0f) states = BLOCK_DIR_WEST;
				else if (norm.x > 0.0f) states = BLOCK_DIR_EAST;
				else if (norm.y > 0.0f) states = BLOCK_DIR_UP;
				else if (norm.y < 0.0f) states = BLOCK_DIR_DOWN;
				else if (norm.z > 0.0f) states = BLOCK_DIR_NORTH;
				else if (norm.z < 0.0f) states = BLOCK_DIR_SOUTH;
			} else if (name == "pane") {
				vec3 vec = camera->dir;
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
		
		Block* block = contentIds->getBlockDef(vox->id);
		if (lclick && block->breakable){
			chunks->set(x,y,z, 0, 0);
			lighting->onBlockSet(x,y,z, 0);
		}
		if (rclick){
			if (block->model != BlockModel::xsprite){
				x = (int)(iend.x)+(int)(norm.x);
				y = (int)(iend.y)+(int)(norm.y);
				z = (int)(iend.z)+(int)(norm.z);
			}
			vox = chunks->get(x, y, z);
			if (vox && (block = contentIds->getBlockDef(vox->id))->replaceable) {
				if (!level->physics->isBlockInside(x,y,z, player->hitbox)){
					chunks->set(x, y, z, player->choosenBlock, states);
					lighting->onBlockSet(x,y,z, player->choosenBlock);
				}
			}
		}
		if (Events::jactive(BIND_PLAYER_PICK)){
			player->choosenBlock = chunks->get(x,y,z)->id;
		}
	} else {
		selectedBlockId = -1;
		selectedBlockStates = 0;
	}
}