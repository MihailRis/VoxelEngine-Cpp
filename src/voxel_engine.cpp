#include <iostream>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <algorithm>
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

bool occlusion = false;

// All in-game definitions (blocks, items, etc..)
void setup_definitions() {
	// AIR
	Block* block = new Block(0,0);
	block->drawGroup = 1;
	block->lightPassing = true;
	block->skyLightPassing = true;
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

	// WOOD
	block = new Block(6,7);
	block->textureFaces[2] = 8;
	block->textureFaces[3] = 8;
	Block::blocks[block->id] = block;

	// LEAVES
	block = new Block(7,9);
	Block::blocks[block->id] = block;

	// ACTUAL STONE
	block = new Block(8,10);
	Block::blocks[block->id] = block;

	// WATER
	block = new Block(9,11);
	block->drawGroup = 4;
	block->lightPassing = true;
	block->skyLightPassing = false;
	block->obstacle = false;
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

void draw_chunk(size_t index, Camera* camera){
	Chunk* chunk = chunks->chunks[index];
	Mesh* mesh = chunks->meshes[index];
	if (mesh == nullptr)
		return;

	// Simple frustum culling (culling chunks behind the camera in 2D - XZ)
	if (occlusion){
		const float cameraX = camera->position.x;
		const float cameraZ = camera->position.z;
		const float camDirX = camera->dir.x;
		const float camDirZ = camera->dir.z;

		bool unoccluded = false;
		do {
			if ((chunk->x*CHUNK_W-cameraX)*camDirX + (chunk->z*CHUNK_D-cameraZ)*camDirZ >= 0.0){
				unoccluded = true; break;
			}
			if (((chunk->x+1)*CHUNK_W-cameraX)*camDirX + (chunk->z*CHUNK_D-cameraZ)*camDirZ >= 0.0){
				unoccluded = true; break;
			}
			if (((chunk->x+1)*CHUNK_W-cameraX)*camDirX + ((chunk->z+1)*CHUNK_D-cameraZ)*camDirZ >= 0.0){
				unoccluded = true; break;
			}
			if ((chunk->x*CHUNK_W-cameraX)*camDirX + ((chunk->z+1)*CHUNK_D-cameraZ)*camDirZ >= 0.0){
				unoccluded = true; break;
			}
		} while (false);
		if (!unoccluded)
			return;
	}

	mat4 model = glm::translate(mat4(1.0f), vec3(chunk->x*CHUNK_W+0.5f, chunk->y*CHUNK_H+0.5f, chunk->z*CHUNK_D+0.5f));
	shader->uniformMatrix("u_model", model);
	mesh->draw(GL_TRIANGLES);
}

float find_most_distant_sqr(float px, float pz, float distance_limit2){
	float max_dist2 = -1.0f;
	for (size_t i = 0; i < chunks->volume; i++){
		Chunk* chunk = chunks->chunks[i];
		if (chunk == nullptr)
			continue;
		float dist2 = (chunk->x - px) * (chunk->z - pz);
		if (dist2 > max_dist2 && dist2 < distance_limit2){
			max_dist2 = dist2;
		}
	}
	return max_dist2;
}

float _camera_cx;
float _camera_cz;

bool chunks_comparator(size_t i, size_t j) {
	Chunk* a = chunks->chunks[i];
	Chunk* b = chunks->chunks[j];
	return ((a->x + 0.5f - _camera_cx)*(a->x + 0.5f - _camera_cx) + (a->z + 0.5f - _camera_cz)*(a->z + 0.5f - _camera_cz)
			>
			(b->x + 0.5f - _camera_cx)*(b->x + 0.5f - _camera_cx) + (b->z + 0.5f - _camera_cz)*(b->z + 0.5f - _camera_cz));
}

void draw_world(Camera* camera){
	glClearColor(0.7f,0.71f,0.73f,1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Draw VAO
	shader->use();
	shader->uniformMatrix("u_proj", camera->getProjection());
	shader->uniformMatrix("u_view", camera->getView());
	shader->uniform1f("u_gamma", 1.6f);
	shader->uniform3f("u_skyLightColor", 1.8f,1.8f,1.8f);
	shader->uniform3f("u_fogColor", 0.7f,0.71f,0.73f);
	shader->uniform3f("u_cameraPos", camera->position.x,camera->position.y,camera->position.z);
	texture->bind();

	std::vector<size_t> indices;

	for (size_t i = 0; i < chunks->volume; i++){
		Chunk* chunk = chunks->chunks[i];
		if (chunk == nullptr)
			continue;
		if (chunks->meshes[i] != nullptr)
			indices.push_back(i);
	}

	std::sort(indices.begin(), indices.end(), chunks_comparator);


	float px = camera->position.x / (float)CHUNK_W;
	float pz = camera->position.z / (float)CHUNK_D;

	_camera_cx = px;
	_camera_cz = pz;


	for (size_t i = 0; i < indices.size(); i++){
		draw_chunk(indices[i], camera);
	}

	crosshairShader->use();
	crosshairShader->uniform1f("u_ar", (float)Window::height / (float)Window::width);
	crosshairShader->uniform1f("u_scale", 1.0f / ((float)Window::height / 1000.0f));
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

	Camera* camera = new Camera(vec3(-320,255,32), radians(90.0f));

	wfile = new WorldFiles("world/", REGION_VOL * (CHUNK_VOL * 2 + 8));
	chunks = new Chunks(34,1,34, 0,0,0);

	float camX = 0.0f;
	float camY = 0.0f;

	wfile->readPlayer(camera->position, camX, camY);
	camera->rotation = mat4(1.0f);
	camera->rotate(camY, camX, 0);

	Hitbox* hitbox = new Hitbox(vec3(camera->position.x,camera->position.y+1,camera->position.z), vec3(0.2f,0.9f,0.2f));

	VoxelRenderer renderer(1024*1024);
	lineBatch = new LineBatch(4096);
	PhysicsSolver physics(vec3(0,-16.0f,0));

	Lighting lighting(chunks);

	crosshair = new Mesh(vertices, 4, attrs);

	ChunksController chunksController(chunks, &lighting);

	float lastTime = glfwGetTime();
	float delta = 0.0f;

	float playerSpeed = 5.0f;

	int choosenBlock = 1;
	long frame = 0;

	glfwSwapInterval(1);

	while (!Window::isShouldClose()){
		frame++;
		float currentTime = glfwGetTime();
		delta = currentTime - lastTime;
		lastTime = currentTime;

		if (frame % 240 == 0)
			std::cout << 1.0/delta << std::endl;

		if (Events::jpressed(GLFW_KEY_O)){
			occlusion = !occlusion;
		}

		if (Events::jpressed(GLFW_KEY_ESCAPE)){
			Window::setShouldClose(true);
		}
		if (Events::jpressed(GLFW_KEY_TAB)){
			Events::toogleCursor();
		}

		for (int i = 1; i < 10; i++){
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
		chunksController._buildMeshes(&renderer, frame);
		chunksController.loadVisible(wfile);

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
					lighting.onBlockSet(x,y,z,0);
				}
				if (Events::jclicked(GLFW_MOUSE_BUTTON_2)){
					int x = (int)(iend.x)+(int)(norm.x);
					int y = (int)(iend.y)+(int)(norm.y);
					int z = (int)(iend.z)+(int)(norm.z);
					if (!physics.isBlockInside(x,y,z, hitbox)){
						chunks->set(x, y, z, choosenBlock);
						lighting.onBlockSet(x,y,z, choosenBlock);
					}
				}
			}
		}
		draw_world(camera);

		Window::swapBuffers();
		Events::pullEvents();
	}

	wfile->writePlayer(hitbox->position, camX, camY);
	write_world();
	close_world();

	finalize_assets();
	Window::terminate();
	return 0;
}
