#include <iostream>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

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
#include "files/files.h"
#include "files/WorldFiles.h"
#include "lighting/LightSolver.h"
#include "lighting/Lightmap.h"
#include "lighting/Lighting.h"
#include "physics/Hitbox.h"
#include "physics/PhysicsSolver.h"

int WIDTH = 1280;
int HEIGHT = 720;

float vertices[] = {
		// x    y
	   -0.01f,-0.01f,
	    0.01f, 0.01f,

	   -0.01f, 0.01f,
	    0.01f,-0.01f,
};

int attrs[] = {
		2,  0 //null terminator
};

Mesh *crosshair;
Shader *shader, *linesShader, *crosshairShader;
Texture *texture;
LineBatch *lineBatch;

Chunks* chunks;
WorldFiles* wfile;

// All in-game definitions (blocks, items, etc..)
void setup_definitions() {
	// AIR
	Block* block = new Block(0,0);
	block->drawGroup = 1;
	block->lightPassing = true;
	block->obstacle = false;
	Block::blocks[block->id] = block;

	// STONE
	block = new Block(1,2);
	Block::blocks[block->id] = block;

	// GRASS
	block = new Block(2,4);
	block->textureFaces[2] = 2;
	block->textureFaces[3] = 1;
	Block::blocks[block->id] = block;

	// LAMP
	block = new Block(3,3);
	block->emission[0] = 15;
	block->emission[1] = 14;
	block->emission[2] = 13;
	Block::blocks[block->id] = block;

	// GLASS
	block = new Block(4,5);
	block->drawGroup = 2;
	block->lightPassing = true;
	Block::blocks[block->id] = block;

	// PLANKS
	block = new Block(5,6);
	Block::blocks[block->id] = block;
}

// Shaders, textures, renderers
int initialize_assets() {
	shader = load_shader("res/main.glslv", "res/main.glslf");
	if (shader == nullptr){
		std::cerr << "failed to load shader" << std::endl;
		Window::terminate();
		return 1;
	}

	crosshairShader = load_shader("res/crosshair.glslv", "res/crosshair.glslf");
	if (crosshairShader == nullptr){
		std::cerr << "failed to load crosshair shader" << std::endl;
		Window::terminate();
		return 1;
	}

	linesShader = load_shader("res/lines.glslv", "res/lines.glslf");
	if (linesShader == nullptr){
		std::cerr << "failed to load lines shader" << std::endl;
		Window::terminate();
		return 1;
	}

	texture = load_texture("res/block.png");
	if (texture == nullptr){
		std::cerr << "failed to load texture" << std::endl;
		delete shader;
		Window::terminate();
		return 1;
	}
	return 0;
}

void draw_world(Camera* camera){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Draw VAO
	shader->use();
	shader->uniformMatrix("u_projview", camera->getProjection()*camera->getView());
	shader->uniform1f("u_gamma", 1.6f);
	shader->uniform3f("u_skyLightColor", 0.1*2,0.15*2,0.2*2);
	texture->bind();
	mat4 model(1.0f);
	for (size_t i = 0; i < chunks->volume; i++){
		Chunk* chunk = chunks->chunks[i];
		if (chunk == nullptr)
			continue;
		Mesh* mesh = chunks->meshes[i];
		if (mesh == nullptr)
			continue;
		model = glm::translate(mat4(1.0f), vec3(chunk->x*CHUNK_W+0.5f, chunk->y*CHUNK_H+0.5f, chunk->z*CHUNK_D+0.5f));
		shader->uniformMatrix("u_model", model);
		mesh->draw(GL_TRIANGLES);
	}

	crosshairShader->use();
	crosshair->draw(GL_LINES);

	linesShader->use();
	linesShader->uniformMatrix("u_projview", camera->getProjection()*camera->getView());
	glLineWidth(2.0f);
	lineBatch->render();
}

// Deleting GL objects like shaders, textures
void finalize_assets(){
	delete shader;
	delete texture;
	delete crosshair;
	delete crosshairShader;
	delete linesShader;
	delete lineBatch;
}

// Save all world data to files
void write_world(){
	for (unsigned int i = 0; i < chunks->volume; i++){
		Chunk* chunk = chunks->chunks[i];
		if (chunk == nullptr)
			continue;
		wfile->put((const char*)chunk->voxels, chunk->x, chunk->z);
	}

	wfile->write();
}

// Deleting world data from memory
void close_world(){
	delete chunks;
	delete wfile;
}

int main() {
	setup_definitions();

	Window::initialize(WIDTH, HEIGHT, "Window 2.0");
	Events::initialize();

	int result = initialize_assets();
	if (result){
		Window::terminate();
		return result;
	}

	wfile = new WorldFiles("world/", REGION_VOL * (CHUNK_VOL * 2 + 8));
	chunks = new Chunks(32,1,32, 0,0,0);
	VoxelRenderer renderer(1024*1024);
	lineBatch = new LineBatch(4096);
	PhysicsSolver physics(vec3(0,-16.0f,0));

	Lighting::initialize(chunks);

	crosshair = new Mesh(vertices, 4, attrs);
	Camera* camera = new Camera(vec3(32,32,32), radians(90.0f));
	Hitbox* hitbox = new Hitbox(vec3(32,32,32), vec3(0.2f,0.9f,0.2f));

	float lastTime = glfwGetTime();
	float delta = 0.0f;

	float camX = 0.0f;
	float camY = 0.0f;

	float playerSpeed = 4.0f;

	int choosenBlock = 1;
	long frame = 0;

	glfwSwapInterval(1);

	while (!Window::isShouldClose()){
		frame++;
		float currentTime = glfwGetTime();
		delta = currentTime - lastTime;
		lastTime = currentTime;

		if (Events::jpressed(GLFW_KEY_ESCAPE)){
			Window::setShouldClose(true);
		}
		if (Events::jpressed(GLFW_KEY_TAB)){
			Events::toogleCursor();
		}

		for (int i = 1; i < 6; i++){
			if (Events::jpressed(GLFW_KEY_0+i)){
				choosenBlock = i;
			}
		}

		// Controls
		bool sprint = Events::pressed(GLFW_KEY_LEFT_CONTROL);
		bool shift = Events::pressed(GLFW_KEY_LEFT_SHIFT) && hitbox->grounded && !sprint;

		float speed = playerSpeed;
		int substeps = (int)(delta * 1000);
		substeps = (substeps <= 0 ? 1 : (substeps > 100 ? 100 : substeps));
		physics.step(chunks, hitbox, delta, substeps, shift);
		camera->position.x = hitbox->position.x;
		camera->position.y = hitbox->position.y + 0.5f;
		camera->position.z = hitbox->position.z;

		float dt = min(1.0f, delta * 16);
		if (shift){
			speed *= 0.25f;
			camera->position.y -= 0.2f;
			camera->zoom = 0.9f * dt + camera->zoom * (1.0f - dt);
		} else if (sprint){
			speed *= 1.5f;
			camera->zoom = 1.1f * dt + camera->zoom * (1.0f - dt);
		} else {
			camera->zoom = dt + camera->zoom * (1.0f - dt);
		}
		if (Events::pressed(GLFW_KEY_SPACE) && hitbox->grounded){
			hitbox->velocity.y = 6.0f;
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
		if (length(dir) > 0.0f)
			dir = normalize(dir);
		hitbox->velocity.x = dir.x * speed;
		hitbox->velocity.z = dir.z * speed;

		chunks->setCenter(wfile, camera->position.x,0,camera->position.z);
		chunks->_buildMeshes(&renderer);
		chunks->loadVisible(wfile);

		if (Events::_cursor_locked){
			camY += -Events::deltaY / Window::height * 2;
			camX += -Events::deltaX / Window::height * 2;

			if (camY < -radians(89.0f)){
				camY = -radians(89.0f);
			}
			if (camY > radians(89.0f)){
				camY = radians(89.0f);
			}

			camera->rotation = mat4(1.0f);
			camera->rotate(camY, camX, 0);
		}

		{
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
					Lighting::onBlockSet(x,y,z,0);
				}
				if (Events::jclicked(GLFW_MOUSE_BUTTON_2)){
					int x = (int)(iend.x)+(int)(norm.x);
					int y = (int)(iend.y)+(int)(norm.y);
					int z = (int)(iend.z)+(int)(norm.z);
					if (!physics.isBlockInside(x,y,z, hitbox)){
						chunks->set(x, y, z, choosenBlock);
						Lighting::onBlockSet(x,y,z, choosenBlock);
					}
				}
			}
		}
		draw_world(camera);

		Window::swapBuffers();
		Events::pullEvents();
	}

	write_world();
	close_world();

	Lighting::finalize();
	finalize_assets();
	Window::terminate();
	return 0;
}
