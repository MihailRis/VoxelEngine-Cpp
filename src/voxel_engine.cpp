// Install dependencies:
// sudo apt install libgl-dev libglew-dev libglfw3-dev libpng-dev libglm-dev
#include <iostream>
#include <cmath>
#include <stdint.h>

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
#include "graphics/Batch2D.h"
#include "graphics/Framebuffer.h"
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
#include "world/World.h"
#include "world/Level.h"

#include "audio/Audio.h"
#include "audio/audioutil.h"
#include "Assets.h"
#include "objects/Player.h"

#include "declarations.h"
#include "world_render.h"
#include "hud_render.h"
#include "player_control.h"

int WIDTH = 1280;
int HEIGHT = 720;

// Save all world data to files
void write_world(World* world, Level* level){
	WorldFiles* wfile = world->wfile;
	Chunks* chunks = level->chunks;

	for (unsigned int i = 0; i < chunks->volume; i++){
		Chunk* chunk = chunks->chunks[i];
		if (chunk == nullptr || !chunk->isUnsaved())
			continue;
		wfile->put((const char*)chunk->voxels, chunk->x, chunk->z);
	}

	wfile->write();

	world->wfile->writePlayer(level->player);
}

void update_level(World* world, Level* level, float delta, long frame, VoxelRenderer* renderer) {
	level->playerController->update_controls(delta);
	if (Events::_cursor_locked)
		level->playerController->update_interaction();

	vec3 position = level->player->hitbox->position;
	level->chunks->setCenter(world->wfile, position.x, position.z);
}

Level* load_level(World* world, Player* player) {
	Level* level = new Level(world, player, new Chunks(56, 56, 0, 0), new PhysicsSolver(vec3(0, -19.6f, 0)));
	world->wfile->readPlayer(player);

	Camera* camera = player->camera;
	camera->rotation = mat4(1.0f);
	camera->rotate(player->camY, player->camX, 0);
	return level;
}

int initialize(Assets*& assets) {
	Window::initialize(WIDTH, HEIGHT, "VoxelEngine-Cpp v12");
	Events::initialize();

	assets = new Assets();
	std::cout << "-- loading assets" << std::endl;
	int result = initialize_assets(assets);
	if (result){
		delete assets;
		Window::terminate();
		return result;
	}
	return 0;
}

void mainloop(Level* level, Assets* assets) {
	Camera* camera = level->player->camera;
	std::cout << "-- preparing systems" << std::endl;
	World* world = level->world;
	WorldRenderer worldRenderer(level, assets);
	HudRenderer hud;
	long frame = 0;
	float lastTime = glfwGetTime();
	float delta = 0.0f;
	bool occlusion = true;
	bool devdata = false;
	Window::swapInterval(1);
	while (!Window::isShouldClose()){
		frame++;
		float currentTime = glfwGetTime();
		delta = currentTime - lastTime;
		lastTime = currentTime;
		int fps = 1 / delta;
		if (Events::jpressed(GLFW_KEY_ESCAPE)){
			Window::setShouldClose(true);
		}
		if (Events::jpressed(GLFW_KEY_TAB) || Events::jpressed(GLFW_KEY_E)){
			Events::toggleCursor();
		}
		if (Events::jpressed(GLFW_KEY_O)){
			occlusion = !occlusion;
		}
		if (Events::jpressed(GLFW_KEY_F3)){
			devdata = !devdata;
		}
		if (Events::jpressed(GLFW_KEY_F5)){
			for (unsigned i = 0; i < level->chunks->volume; i++) {
				Chunk* chunk = level->chunks->chunks[i];
				if (chunk != nullptr && chunk->isReady()){
					chunk->setModified(true);
				}
			}
		}

		update_level(world, level, delta, frame, worldRenderer.renderer);
		int freeLoaders = level->chunksController->countFreeLoaders();
		for (int i = 0; i < freeLoaders; i++)
			level->chunksController->_buildMeshes(worldRenderer.renderer, frame);
		freeLoaders = level->chunksController->countFreeLoaders();
		for (int i = 0; i < freeLoaders; i++)
			level->chunksController->calculateLights();
		freeLoaders = level->chunksController->countFreeLoaders();
		for (int i = 0; i < freeLoaders; i++)
			level->chunksController->loadVisible(world->wfile);

		worldRenderer.draw(world, camera, occlusion, devdata);
		hud.draw(level, assets, devdata);
		if (devdata) {
			hud.drawDebug(level, assets, fps, occlusion);
		}

		Window::swapBuffers();
		Events::pullEvents();
	}
}

int main() {
	setup_definitions();

	Assets* assets;
	int status = initialize(assets);
	if (status) return status;

	std::cout << "-- loading world" << std::endl;
	vec3 playerPosition = vec3(0,64,0);
	Camera* camera = new Camera(playerPosition, radians(90.0f));
	World* world = new World("world-1", "world/", 42);
	Player* player = new Player(playerPosition, 4.0f, camera);
	Level* level = load_level(world, player);

	std::cout << "-- initializing finished" << std::endl;

	Audio::initialize();
	mainloop(level, assets);
	Audio::finalize();

	std::cout << "-- saving world" << std::endl;
	write_world(world, level);

	delete level;
	delete world;

	std::cout << "-- shutting down" << std::endl;
	delete assets;
	Events::finalize();
	Window::terminate();
	return 0;
}
