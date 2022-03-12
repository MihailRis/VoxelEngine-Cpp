#ifndef WORLD_RENDERER_CPP
#define WORLD_RENDERER_CPP

#include <vector>
#include <algorithm>
#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "window/Window.h"
#include "window/Camera.h"
#include "graphics/Mesh.h"
#include "graphics/Shader.h"
#include "graphics/Texture.h"
#include "graphics/LineBatch.h"
#include "voxels/Chunks.h"
#include "voxels/Chunk.h"

float _camera_cx;
float _camera_cz;
Chunks* _chunks;

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

LineBatch *lineBatch;

void init_renderer(){
	crosshair = new Mesh(vertices, 4, attrs);
	lineBatch = new LineBatch(4096);
}


void finalize_renderer(){
	delete crosshair;
	delete lineBatch;
}

void draw_chunk(size_t index, Camera* camera, Shader* shader, bool occlusion){
	Chunk* chunk = _chunks->chunks[index];
	Mesh* mesh = _chunks->meshes[index];
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

bool chunks_comparator(size_t i, size_t j) {
	Chunk* a = _chunks->chunks[i];
	Chunk* b = _chunks->chunks[j];
	return ((a->x + 0.5f - _camera_cx)*(a->x + 0.5f - _camera_cx) + (a->z + 0.5f - _camera_cz)*(a->z + 0.5f - _camera_cz)
			>
			(b->x + 0.5f - _camera_cx)*(b->x + 0.5f - _camera_cx) + (b->z + 0.5f - _camera_cz)*(b->z + 0.5f - _camera_cz));
}


void draw_world(Camera* camera, Assets* assets,
				Chunks* chunks, bool occlusion){
	glClearColor(0.7f,0.71f,0.73f,1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	_chunks = chunks;

	// Draw VAO
	Texture* texture = assets->getTexture("block");
	Shader* shader = assets->getShader("main");
	Shader* crosshairShader = assets->getShader("crosshair");
	Shader* linesShader = assets->getShader("lines");
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

	float px = camera->position.x / (float)CHUNK_W;
	float pz = camera->position.z / (float)CHUNK_D;

	_camera_cx = px;
	_camera_cz = pz;

	std::sort(indices.begin(), indices.end(), chunks_comparator);


	for (size_t i = 0; i < indices.size(); i++){
		draw_chunk(indices[i], camera, shader, occlusion);
	}

	crosshairShader->use();
	crosshairShader->uniform1f("u_ar", (float)Window::height / (float)Window::width);
	crosshairShader->uniform1f("u_scale", 1.0f / ((float)Window::height / 1000.0f));
	crosshair->draw(GL_LINES);

	linesShader->use();
	linesShader->uniformMatrix("u_projview", camera->getProjection()*camera->getView());
	glLineWidth(2.0f);
	lineBatch->line(camera->position.x, camera->position.y-0.5f, camera->position.z, camera->position.x+0.1f, camera->position.y-0.5f, camera->position.z, 1, 0, 0, 1);
	lineBatch->line(camera->position.x, camera->position.y-0.5f, camera->position.z, camera->position.x, camera->position.y-0.5f, camera->position.z+0.1f, 0, 0, 1, 1);
	lineBatch->render();
}

#endif // WORLD_RENDERER_CPP
