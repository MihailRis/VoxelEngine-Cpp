#include "world_render.h"

#include "graphics/VoxelRenderer.h"

#include "window/Window.h"
#include "window/Camera.h"
#include "graphics/Mesh.h"
#include "graphics/Shader.h"
#include "graphics/Texture.h"
#include "graphics/LineBatch.h"
#include "graphics/Batch3D.h"
#include "voxels/Chunks.h"
#include "voxels/Chunk.h"
#include "world/World.h"
#include "world/Level.h"
#include "Assets.h"

float _camera_cx;
float _camera_cz;

Mesh *crosshair;

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

WorldRenderer::WorldRenderer(Level* level) {
	this->level = level;
	crosshair = new Mesh(vertices, 4, attrs);
	lineBatch = new LineBatch(4096);
	batch3d = new Batch3D(1024);
	renderer = new VoxelRenderer();
}

WorldRenderer::~WorldRenderer() {
	delete crosshair;
	delete lineBatch;
	delete renderer;
}

Chunks* _chunks = nullptr;

bool chunks_distance_compare(size_t i, size_t j) {
	Chunks* chunks = _chunks;
	Chunk* a = chunks->chunks[i];
	Chunk* b = chunks->chunks[j];
	return ((a->x + 0.5f - _camera_cx)*(a->x + 0.5f - _camera_cx) + (a->z + 0.5f - _camera_cz)*(a->z + 0.5f - _camera_cz)
			>
			(b->x + 0.5f - _camera_cx)*(b->x + 0.5f - _camera_cx) + (b->z + 0.5f - _camera_cz)*(b->z + 0.5f - _camera_cz));
}

void WorldRenderer::drawChunk(size_t index, Camera* camera, Shader* shader, bool occlusion){
	Chunk* chunk = level->chunks->chunks[index];
	Mesh* mesh = level->chunks->meshes[index];
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

void WorldRenderer::draw(World* world, Camera* camera, Assets* assets, bool occlusion){
	Chunks* chunks = level->chunks;

	glClearColor(0.7f,0.81f,1.0f,1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	Texture* texture = assets->getTexture("block");
	Shader* shader = assets->getShader("main");
	Shader* crosshairShader = assets->getShader("crosshair");
	Shader* linesShader = assets->getShader("lines");
	shader->use();
	shader->uniformMatrix("u_proj", camera->getProjection());
	shader->uniformMatrix("u_view", camera->getView());
	shader->uniform1f("u_gamma", 1.6f);
	shader->uniform3f("u_skyLightColor", 2.2f,2.2f,2.2f);
	shader->uniform3f("u_fogColor", 0.7f,0.81f,1.0f);
	shader->uniform1f("u_fogFactor", 0.03f);
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

	float px = camera->position.x / (float)CHUNK_W;
	float pz = camera->position.z / (float)CHUNK_D;

	_camera_cx = px;
	_camera_cz = pz;
	_chunks = chunks;
	std::sort(indices.begin(), indices.end(), chunks_distance_compare);


	for (size_t i = 0; i < indices.size(); i++){
		drawChunk(indices[i], camera, shader, occlusion);
	}

	shader->uniformMatrix("u_model", mat4(1.0f));
	batch3d->begin();
	// draw 3D stuff here
	batch3d->render();

	crosshairShader->use();
	crosshairShader->uniform1f("u_ar", (float)Window::height / (float)Window::width);
	crosshairShader->uniform1f("u_scale", 1.0f / ((float)Window::height / 1000.0f));
	crosshair->draw(GL_LINES);

	linesShader->use();
	linesShader->uniformMatrix("u_projview", camera->getProjection()*camera->getView());
	glLineWidth(2.0f);
	lineBatch->line(camera->position.x, camera->position.y-0.1f, camera->position.z, camera->position.x+0.01f, camera->position.y-0.1f, camera->position.z, 1, 0, 0, 1);
	lineBatch->line(camera->position.x, camera->position.y-0.1f, camera->position.z, camera->position.x, camera->position.y-0.1f, camera->position.z+0.01f, 0, 0, 1, 1);
	lineBatch->line(camera->position.x, camera->position.y-0.1f, camera->position.z, camera->position.x, camera->position.y-0.1f+0.01f, camera->position.z, 0, 1, 0, 1);
	lineBatch->render();
}
