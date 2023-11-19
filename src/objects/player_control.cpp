#include "player_control.h"

#include "Player.h"
#include "../physics/PhysicsSolver.h"
#include "../physics/Hitbox.h"
#include "../lighting/Lighting.h"
#include "../world/Level.h"
#include "../voxels/Block.h"
#include "../voxels/voxel.h"
#include "../voxels/Chunks.h"
#include "../window/Camera.h"
#include "../window/Events.h"
#include "../window/input.h"

#define CROUCH_SPEED_MUL 0.25f
#define CROUCH_SHIFT_Y -0.2f
#define RUN_SPEED_MUL 1.5f
#define CROUCH_ZOOM 0.9f
#define RUN_ZOOM 1.1f
#define C_ZOOM 0.1f
#define ZOOM_SPEED 16.0f
#define PLAYER_GROUND_DAMPING 10.0f
#define PLAYER_AIR_DAMPING 7.0f
#define CAMERA_SHAKING_OFFSET 0.025f
#define CAMERA_SHAKING_OFFSET_Y 0.031f
#define CAMERA_SHAKING_SPEED 1.6f
#define CAMERA_SHAKING_DELTA_K 10.0f
#define FLIGHT_SPEED_MUL 4.0f
#define CHEAT_SPEED_MUL 5.0f
#define JUMP_FORCE 7.0f

PlayerController::PlayerController(Level* level, const EngineSettings& settings) 
	: level(level), player(level->player), camSettings(settings.camera) {
}

void PlayerController::refreshCamera() {
	level->player->camera->position = level->player->hitbox->position + cameraOffset;
}

void PlayerController::updateKeyboard() {
	input.zoom = Events::pressed(keycode::C);
	input.moveForward = Events::pressed(keycode::W);
	input.moveBack = Events::pressed(keycode::S);
	input.moveLeft = Events::pressed(keycode::A);
	input.moveRight = Events::pressed(keycode::D);
	input.sprint = Events::pressed(keycode::LEFT_CONTROL);
	input.shift = Events::pressed(keycode::LEFT_SHIFT);
	input.cheat = Events::pressed(keycode::R);
	input.jump = Events::pressed(keycode::SPACE);

	input.noclip = Events::jpressed(keycode::N);
	input.flight = Events::jpressed(keycode::F);

	// block choice
	for (int i = 1; i < 10; i++){
		if (Events::jpressed(keycode::NUM_0+i)){
			player->choosenBlock = i;
		}
	}
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
	Player* player = level->player;
	Camera* camera = player->camera;
	Hitbox* hitbox = player->hitbox;

	bool crouch = input.shift && hitbox->grounded && !input.sprint;
	float speed = player->speed;

	if (player->flight){
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

	vec3 dir(0,0,0);
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
	if (length(dir) > 0.0f){
		dir = normalize(dir);
		hitbox->velocity.x += dir.x * speed * delta * 9;
		hitbox->velocity.z += dir.z * speed * delta * 9;
	}

	int substeps = (int)(delta * 1000);
	substeps = (substeps <= 0 ? 1 : (substeps > 100 ? 100 : substeps));
	level->physics->step(level->chunks, hitbox, delta, substeps, crouch, player->flight ? 0.0f : 1.0f, !player->noclip);
	if (player->flight && hitbox->grounded) {
		player->flight = false;
	}

	if (input.jump && hitbox->grounded){
		hitbox->velocity.y = JUMP_FORCE;
	}

	cameraOffset = vec3(0.0f, 0.7f, 0.0f);

	if (camSettings.shaking) {
		player->interpVel = player->interpVel * (1.0f - delta * 5) + hitbox->velocity * delta * 0.1f;
		if (hitbox->grounded && player->interpVel.y < 0.0f){
			player->interpVel.y *= -30.0f;
		}
		float factor = hitbox->grounded ? length(vec2(hitbox->velocity.x, hitbox->velocity.z)) : 0.0f;
		player->cameraShakingTimer += delta * factor * CAMERA_SHAKING_SPEED;
		float shakeTimer = player->cameraShakingTimer;
		player->cameraShaking = player->cameraShaking * (1.0f - delta * CAMERA_SHAKING_DELTA_K) + factor * delta * CAMERA_SHAKING_DELTA_K;
		cameraOffset += camera->right * sin(shakeTimer) * CAMERA_SHAKING_OFFSET * player->cameraShaking;
		cameraOffset += camera->up * abs(cos(shakeTimer)) * CAMERA_SHAKING_OFFSET_Y * player->cameraShaking;
		cameraOffset -= min(player->interpVel * 0.05f, 1.0f);
	}

	if ((input.flight && !player->noclip) ||
		(input.noclip && player->flight == player->noclip)){
		player->flight = !player->flight;
		if (player->flight){
			hitbox->grounded = false;
		}
	}
	if (input.noclip) {
		player->noclip = !player->noclip;
	}

	if (camSettings.fovEvents){
		float dt = min(1.0f, delta * ZOOM_SPEED);
		float zoomValue = 1.0f;
		if (crouch){
			cameraOffset += vec3(0.f, CROUCH_SHIFT_Y, 0.f);
			zoomValue = CROUCH_ZOOM;
		} else if (input.sprint){
			zoomValue = RUN_ZOOM;
		}
		if (input.zoom)
			zoomValue *= C_ZOOM;
		camera->zoom = zoomValue * dt + camera->zoom * (1.0f - dt);
	}

	hitbox->linear_damping = PLAYER_GROUND_DAMPING;
	if (player->flight){
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
}

void PlayerController::updateCameraControl() {
	Camera* camera = player->camera;
	float rotX = -Events::deltaX / Window::height * 2;
	float rotY = -Events::deltaY / Window::height * 2;
	if (input.zoom){
		rotX /= 4;
		rotY /= 4;
	}
	player->camX += rotX;
	player->camY += rotY;

	if (player->camY < -radians(89.9f)){
		player->camY = -radians(89.9f);
	}
	if (player->camY > radians(89.9f)){
		player->camY = radians(89.9f);
	}

	camera->rotation = mat4(1.0f);
	camera->rotate(player->camY, player->camX, 0);
}

void PlayerController::updateInteraction(){
	Chunks* chunks = level->chunks;
	Player* player = level->player;
	Lighting* lighting = level->lighting;
	Camera* camera = player->camera;
	vec3 end;
	vec3 norm;

	bool xkey = Events::pressed(keycode::X);
	bool lclick = Events::jclicked(mousecode::BUTTON_1) || 
				  (xkey && Events::clicked(mousecode::BUTTON_1));
	bool rclick = Events::jclicked(mousecode::BUTTON_2) || 
				  (xkey && Events::clicked(mousecode::BUTTON_2));
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
		selectedBlockPosition = iend;
		int x = (int)iend.x;
		int y = (int)iend.y;
		int z = (int)iend.z;
		uint8_t states = 0;

		if (Block::blocks[player->choosenBlock]->rotatable){
			if (abs(norm.x) > abs(norm.z)){
				if (abs(norm.x) > abs(norm.y)) states = BLOCK_DIR_X;
				if (abs(norm.x) < abs(norm.y)) states = BLOCK_DIR_Y;
			}
			if (abs(norm.x) < abs(norm.z)){
				if (abs(norm.z) > abs(norm.y)) states = BLOCK_DIR_Z;
				if (abs(norm.z) < abs(norm.y)) states = BLOCK_DIR_Y;
			}
		}
		
		Block* block = Block::blocks[vox->id];
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
			if (!level->physics->isBlockInside(x,y,z, player->hitbox)){
				chunks->set(x, y, z, player->choosenBlock, states);
				lighting->onBlockSet(x,y,z, player->choosenBlock);
			}
		}
		if (Events::jclicked(mousecode::BUTTON_3)){
			player->choosenBlock = chunks->get(x,y,z)->id;
		}
	} else {
		selectedBlockId = -1;
	}
}
