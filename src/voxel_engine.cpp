// Install dependencies:
// sudo apt install libgl-dev libglew-dev libglfw3-dev libpng-dev libglm-dev
#include <iostream>
#include <cmath>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <vector>
#include <ctime>

// GLM
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

#include "graphics/Shader.h"
#include "graphics/Texture.h"
#include "graphics/Mesh.h"
#include "graphics/VoxelRenderer.h"
#include "graphics/LineBatch.h"
#include "window/Window.h"
#include "window/Events.h"
#include "window/Camera.h"
#include "loaders/png_loading.h"
#include "voxels/voxel.h"
#include "voxels/Chunk.h"
#include "voxels/Chunks.h"
#include "voxels/Block.h"
#include "voxels/WorldGenerator.h"
#include "voxels/ChunksController.h"
#include "files/files.h"
#include "files/WorldFiles.h"
#include "lighting/LightSolver.h"
#include "lighting/Lightmap.h"
#include "lighting/Lighting.h"
#include "physics/Hitbox.h"
#include "physics/PhysicsSolver.h"

#include "Assets.h"
#include "objects/Player.h"

#include "declarations.h"
#include "world_render.h"


// Save all world data to files
void write_world(WorldFiles* wfile, Chunks* chunks){
	for (unsigned int i = 0; i < chunks->volume; i++){
		Chunk* chunk = chunks->chunks[i];
		if (chunk == nullptr)
			continue;
		wfile->put((const char*)chunk->voxels, chunk->x, chunk->z);
	}

	wfile->write();
}

// Deleting world data from memory
void close_world(WorldFiles* wfile, Chunks* chunks){
	delete chunks;
	delete wfile;
}

#define CROUCH_SPEED_MUL 0.25f
#define CROUCH_SHIFT_Y -0.2f
#define RUN_SPEED_MUL 1.5f
#define CROUCH_ZOOM 0.9f
#define RUN_ZOOM 1.1f
#define C_ZOOM 0.1f
#define ZOOM_SPEED 16.0f
#define DEFAULT_AIR_DAMPING 0.1f
#define PLAYER_NOT_ONGROUND_DAMPING 10.0f
#define CAMERA_SHAKING_OFFSET 0.025f
#define CAMERA_SHAKING_OFFSET_Y 0.031f
#define CAMERA_SHAKING_SPEED 1.6f
#define CAMERA_SHAKING_DELTA_K 10.0f
#define FLIGHT_SPEED_MUL 5.0f
#define JUMP_FORCE 7.0f

void update_controls(PhysicsSolver* physics,
		Chunks* chunks,
		Player* player,
		float delta){

	if (Events::jpressed(GLFW_KEY_TAB)){
		Events::toogleCursor();
	}

	for (int i = 1; i < 10; i++){
		if (Events::jpressed(GLFW_KEY_0+i)){
			player->choosenBlock = i;
		}
	}

	// Controls
	Camera* camera = player->camera;
	Hitbox* hitbox = player->hitbox;
	bool sprint = Events::pressed(GLFW_KEY_LEFT_CONTROL);
	bool shift = Events::pressed(GLFW_KEY_LEFT_SHIFT) && hitbox->grounded && !sprint;
	bool zoom = Events::pressed(GLFW_KEY_C);

	float speed = player->speed;
	if (player->flight){
		speed *= FLIGHT_SPEED_MUL;
	}
	int substeps = (int)(delta * 1000);
	substeps = (substeps <= 0 ? 1 : (substeps > 100 ? 100 : substeps));
	physics->step(chunks, hitbox, delta, substeps, shift, player->flight ? 0.0f : 1.0f);
	camera->position.x = hitbox->position.x;
	camera->position.y = hitbox->position.y + 0.5f;
	camera->position.z = hitbox->position.z;

	if (player->flight && hitbox->grounded)
		player->flight = false;
	// Camera shaking
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

	if (Events::jpressed(GLFW_KEY_F)){
		player->flight = !player->flight;
		if (player->flight){
			hitbox->velocity.y += 1;
			hitbox->grounded = false;
		}
	}

	// Field of view manipulations
	float dt = min(1.0f, delta * ZOOM_SPEED);
	if (dt > 1.0f)
		dt = 1.0f;
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

	if (Events::pressed(GLFW_KEY_SPACE) && hitbox->grounded){
		hitbox->velocity.y = JUMP_FORCE;
	}

	vec3 dir(0,0,0);
	if (Events::pressed(GLFW_KEY_W)){
		dir.x += camera->dir.x;
		dir.z += camera->dir.z;
	}
	if (Events::pressed(GLFW_KEY_S)){
		dir.x -= camera->dir.x;
		dir.z -= camera->dir.z;
	}
	if (Events::pressed(GLFW_KEY_D)){
		dir.x += camera->right.x;
		dir.z += camera->right.z;
	}
	if (Events::pressed(GLFW_KEY_A)){
		dir.x -= camera->right.x;
		dir.z -= camera->right.z;
	}

	hitbox->linear_damping = DEFAULT_AIR_DAMPING;
	if (player->flight){
		hitbox->linear_damping = PLAYER_NOT_ONGROUND_DAMPING;
		hitbox->velocity.y *= 1.0f - delta * 9;
		if (Events::pressed(GLFW_KEY_SPACE)){
			hitbox->velocity.y += speed * delta * 9;
		}
		if (Events::pressed(GLFW_KEY_LEFT_SHIFT)){
			hitbox->velocity.y -= speed * delta * 9;
		}
	}
	if (length(dir) > 0.0f){
		dir = normalize(dir);

		if (!hitbox->grounded)
			hitbox->linear_damping = PLAYER_NOT_ONGROUND_DAMPING;

		hitbox->velocity.x += dir.x * speed * delta * 9;
		hitbox->velocity.z += dir.z * speed * delta * 9;
	}

	if (Events::_cursor_locked){
		player->camY += -Events::deltaY / Window::height * 2;
		player->camX += -Events::deltaX / Window::height * 2;

		if (player->camY < -radians(89.0f)){
			player->camY = -radians(89.0f);
		}
		if (player->camY > radians(89.0f)){
			player->camY = radians(89.0f);
		}

		camera->rotation = mat4(1.0f);
		camera->rotate(player->camY, player->camX, 0);
	}
}

void update_interaction(Chunks* chunks, PhysicsSolver* physics, Player* player, Lighting* lighting){
	Camera* camera = player->camera;
	vec3 end;
	vec3 norm;
	vec3 iend;
	voxel* vox = chunks->rayCast(camera->position, camera->front, 10.0f, end, norm, iend);
	if (vox != nullptr){
		lineBatch->box(iend.x+0.5f, iend.y+0.5f, iend.z+0.5f, 1.005f,1.005f,1.005f, 0,0,0,0.5f);

		if (Events::jclicked(GLFW_MOUSE_BUTTON_1)){
			int x = (int)iend.x;
			int y = (int)iend.y;
			int z = (int)iend.z;
			chunks->set(x,y,z, 0);
			lighting->onBlockSet(x,y,z,0);
		}
		if (Events::jclicked(GLFW_MOUSE_BUTTON_2)){
			int x = (int)(iend.x)+(int)(norm.x);
			int y = (int)(iend.y)+(int)(norm.y);
			int z = (int)(iend.z)+(int)(norm.z);
			if (!physics->isBlockInside(x,y,z, player->hitbox)){
				chunks->set(x, y, z, player->choosenBlock);
				lighting->onBlockSet(x,y,z, player->choosenBlock);
			}
		}
	}
}

int WIDTH = 1280;
int HEIGHT = 720;

#define GRAVITY 19.6f
#define DEFAULT_PLAYER_SPEED 4.0f

vec3 spawnpoint(-320, 255, 32);


int main() {
	setup_definitions();

	Window::initialize(WIDTH, HEIGHT, "VoxelEngine Part-11");
	Events::initialize();

	std::cout << "-- loading assets" << std::endl;
	Assets* assets = new Assets();
	int result = initialize_assets(assets);
	if (result){
		delete assets;
		Window::terminate();
		return result;
	}
	std::cout << "-- loading world" << std::endl;

	Camera *camera = new Camera(spawnpoint, radians(90.0f));
	WorldFiles *wfile = new WorldFiles("world/", REGION_VOL * (CHUNK_VOL * 2 + 8));
	Chunks *chunks = new Chunks(34,1,34, 0,0,0);


	Player* player = new Player(vec3(camera->position), DEFAULT_PLAYER_SPEED, camera);
	wfile->readPlayer(player);
	camera->rotation = mat4(1.0f);
	camera->rotate(player->camY, player->camX, 0);

	std::cout << "-- preparing systems" << std::endl;

	VoxelRenderer renderer(1024*1024);
	PhysicsSolver physics(vec3(0,-GRAVITY,0));
	Lighting lighting(chunks);

	init_renderer();

	ChunksController chunksController(chunks, &lighting);

	float lastTime = glfwGetTime();
	float delta = 0.0f;

	long frame = 0;

	bool occlusion = false;

	glfwSwapInterval(1);

	std::cout << "-- initializing finished" << std::endl;

	while (!Window::isShouldClose()){
		frame++;
		float currentTime = glfwGetTime();
		delta = currentTime - lastTime;
		lastTime = currentTime;

		if (Events::jpressed(GLFW_KEY_ESCAPE)){
			Window::setShouldClose(true);
		}

		if (Events::jpressed(GLFW_KEY_O)){
			occlusion = !occlusion;
		}

		update_controls(&physics, chunks, player, delta);
		update_interaction(chunks, &physics, player, &lighting);

		chunks->setCenter(wfile, camera->position.x,0,camera->position.z);
		chunksController._buildMeshes(&renderer, frame);

		int freeLoaders = chunksController.countFreeLoaders();
		for (int i = 0; i < freeLoaders; i++)
			chunksController.loadVisible(wfile);

		draw_world(camera, assets, chunks, occlusion);

		Window::swapBuffers();
		Events::pullEvents();
	}
	std::cout << "-- saving world" << std::endl;

	wfile->writePlayer(player);
	write_world(wfile, chunks);
	close_world(wfile, chunks);

	std::cout << "-- shutting down" << std::endl;

	delete assets;
	finalize_renderer();
	Events::finalize();
	Window::terminate();
	return 0;
}

