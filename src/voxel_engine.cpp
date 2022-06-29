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

#include "audio/Audio.h"
#include "audio/audioutil.h"
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

int WIDTH = 1280;
int HEIGHT = 720;

#include "player_control.h"


int main() {
	setup_definitions();

	Audio::initialize();

	Window::initialize(WIDTH, HEIGHT, "Window 2.0");
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

	Camera *camera = new Camera(vec3(-320,255,32), radians(90.0f));
	WorldFiles *wfile = new WorldFiles("world/", REGION_VOL * (CHUNK_VOL * 2 + 8));
	Chunks *chunks = new Chunks(34,1,34, 0,0,0);


	Player* player = new Player(vec3(camera->position), 4.0f, camera);
	wfile->readPlayer(player);
	camera->rotation = mat4(1.0f);
	camera->rotate(player->camY, player->camX, 0);

	std::cout << "-- preparing systems" << std::endl;

	VoxelRenderer renderer(1024*1024);
	PhysicsSolver physics(vec3(0,-9.8f*2.0f,0));
	Lighting lighting(chunks);

	init_renderer();

	ChunksController chunksController(chunks, &lighting);

	float lastTime = glfwGetTime();
	float delta = 0.0f;

	long frame = 0;

	bool occlusion = false;
	bool devdata = false;

	glfwSwapInterval(0);

	std::cout << "-- initializing finished" << std::endl;

	while (!Window::isShouldClose()){
		frame++;
		float currentTime = glfwGetTime();
		delta = currentTime - lastTime;
		lastTime = currentTime;
		int fps = 1 / delta;

		if (Events::jpressed(GLFW_KEY_O)){
			occlusion = !occlusion;
		}
		if (Events::jpressed(GLFW_KEY_F3)){
			devdata = !devdata;
		}

		update_controls(&physics, chunks, player, delta);
		update_interaction(chunks, &physics, player, &lighting, lineBatch);

		chunks->setCenter(wfile, camera->position.x,0,camera->position.z);
		chunksController._buildMeshes(&renderer, frame);

		int freeLoaders = chunksController.countFreeLoaders();
		for (int i = 0; i < freeLoaders; i++)
			chunksController.loadVisible(wfile);

		draw_world(player, camera, assets, chunks, occlusion);
		draw_hud(player, assets, chunks, devdata, fps);

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
	Audio::finalize();
	Events::finalize();
	Window::terminate();
	return 0;
}
