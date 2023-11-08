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

PlayerController::PlayerController(Level* level) : level(level) {
}

void PlayerController::update_controls(float delta){
	Player* player = level->player;

	/*block choose*/{
		for (int i = 1; i < 10; i++){
			if (Events::jpressed(keycode::NUM_0+i)){
				player->choosenBlock = i;
			}
		}
	}//end

	Camera* camera = player->camera;
	Hitbox* hitbox = player->hitbox;
	bool sprint = Events::pressed(keycode::LEFT_CONTROL);
	bool shift = Events::pressed(keycode::LEFT_SHIFT) && hitbox->grounded && !sprint;
	bool zoom = Events::pressed(keycode::C);
	bool cheat = Events::pressed(keycode::R);

	float speed = player->speed;
	if (player->flight){
		speed *= FLIGHT_SPEED_MUL;
	}
	if (cheat){
		speed *= CHEAT_SPEED_MUL;
	}
	int substeps = (int)(delta * 1000);
	substeps = (substeps <= 0 ? 1 : (substeps > 100 ? 100 : substeps));
	level->physics->step(level->chunks, hitbox, delta, substeps, shift, player->flight ? 0.0f : 1.0f, !player->noclip);
	camera->position.x = hitbox->position.x;
	camera->position.y = hitbox->position.y + 0.7f;
	camera->position.z = hitbox->position.z;

	if (player->flight && hitbox->grounded)
		player->flight = false;
	
	/*camera shaking*/{
		player->interpVel = player->interpVel * (1.0f - delta * 5) + hitbox->velocity * delta * 0.1f;
		if (hitbox->grounded && player->interpVel.y < 0.0f){
			player->interpVel.y *= -30.0f;
		}
		float factor = hitbox->grounded ? length(vec2(hitbox->velocity.x, hitbox->velocity.z)) : 0.0f;
		player->cameraShakingTimer += delta * factor * CAMERA_SHAKING_SPEED;
		float shakeTimer = player->cameraShakingTimer;
		player->cameraShaking = player->cameraShaking * (1.0f - delta * CAMERA_SHAKING_DELTA_K) + factor * delta * CAMERA_SHAKING_DELTA_K;
		camera->position += camera->right * sin(shakeTimer) * CAMERA_SHAKING_OFFSET * player->cameraShaking;
		camera->position += camera->up * abs(cos(shakeTimer)) * CAMERA_SHAKING_OFFSET_Y * player->cameraShaking;
		camera->position -= min(player->interpVel * 0.05f, 1.0f);
	}//end

	if ((Events::jpressed(keycode::F) && !player->noclip) ||
		(Events::jpressed(keycode::N) && player->flight == player->noclip)){
		player->flight = !player->flight;
		if (player->flight){
			hitbox->grounded = false;
		}
	}
	if (Events::jpressed(keycode::N)) {
		player->noclip = !player->noclip;
	}

	/*field of view manipulations*/{
		float dt = min(1.0f, delta * ZOOM_SPEED);
		float zoomValue = 1.0f;
		if (shift){
			speed *= CROUCH_SPEED_MUL;
			camera->position.y += CROUCH_SHIFT_Y;
			zoomValue = CROUCH_ZOOM;
		} else if (sprint){
			speed *= RUN_SPEED_MUL;
			zoomValue = RUN_ZOOM;
		}
		if (zoom)
			zoomValue *= C_ZOOM;
		camera->zoom = zoomValue * dt + camera->zoom * (1.0f - dt);
	}//end

	if (Events::pressed(keycode::SPACE) && hitbox->grounded){
		hitbox->velocity.y = JUMP_FORCE;
	}
	vec3 dir(0,0,0);
	if (Events::pressed(keycode::W)){
		dir.x += camera->dir.x;
		dir.z += camera->dir.z;
	}
	if (Events::pressed(keycode::S)){
		dir.x -= camera->dir.x;
		dir.z -= camera->dir.z;
	}
	if (Events::pressed(keycode::D)){
		dir.x += camera->right.x;
		dir.z += camera->right.z;
	}
	if (Events::pressed(keycode::A)){
		dir.x -= camera->right.x;
		dir.z -= camera->right.z;
	}

	hitbox->linear_damping = PLAYER_GROUND_DAMPING;
	if (player->flight){
		hitbox->linear_damping = PLAYER_AIR_DAMPING;
		hitbox->velocity.y *= 1.0f - delta * 9;
		if (Events::pressed(keycode::SPACE)){
			hitbox->velocity.y += speed * delta * 9;
		}
		if (Events::pressed(keycode::LEFT_SHIFT)){
			hitbox->velocity.y -= speed * delta * 9;
		}
	}
	if (!hitbox->grounded)
		hitbox->linear_damping = PLAYER_AIR_DAMPING;
	if (length(dir) > 0.0f){
		dir = normalize(dir);
		hitbox->velocity.x += dir.x * speed * delta * 9;
		hitbox->velocity.z += dir.z * speed * delta * 9;
	}


	/*camera rotate*/{
		if (Events::_cursor_locked){
			float rotX = -Events::deltaX / Window::height * 2;
			float rotY = -Events::deltaY / Window::height * 2;
			if (zoom){
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
	}//end
}

void PlayerController::update_interaction(){
	Chunks* chunks = level->chunks;
	Player* player = level->player;
	Lighting* lighting = level->lighting;
	Camera* camera = player->camera;
	vec3 end;
	vec3 norm;
	vec3 iend;
	voxel* vox = chunks->rayCast(camera->position, camera->front, 10.0f, end, norm, iend);
	if (vox != nullptr){
		player->selectedVoxel = *vox;
		selectedBlockId = vox->id;
		selectedBlockPosition = iend;
		int x = (int)iend.x;
		int y = (int)iend.y;
		int z = (int)iend.z;
		uint8_t states = 0;

		if (Block::blocks[player->choosenBlock]->rotatable){
			// states = states & 0b11111100;
			// if (abs(norm.x) > abs(norm.z)){
			// 	if (abs(norm.x) > abs(norm.y)) states = states | 0b00000001;
			// 	if (abs(norm.x) < abs(norm.y)) states = states | 0b00000010;
			// }
			// if (abs(norm.x) < abs(norm.z)){
			// 	if (abs(norm.z) > abs(norm.y)) states = states | 0b00000011;
			// 	if (abs(norm.z) < abs(norm.y)) states = states | 0b00000010;
			// }
			if (abs(norm.x) > abs(norm.z)){
				if (abs(norm.x) > abs(norm.y)) states = 0x31;
				if (abs(norm.x) < abs(norm.y)) states = 0x32;
			}
			if (abs(norm.x) < abs(norm.z)){
				if (abs(norm.z) > abs(norm.y)) states = 0x33;
				if (abs(norm.z) < abs(norm.y)) states = 0x32;
			}
		}
		
		Block* block = Block::blocks[vox->id];
		if (Events::jclicked(mousecode::BUTTON_1) && block->breakable){
			chunks->set(x,y,z, 0, 0);
			lighting->onBlockSet(x,y,z, 0);
		}
		if (Events::jclicked(mousecode::BUTTON_2)){
			if (block->model != BLOCK_MODEL_X_SPRITE){
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
