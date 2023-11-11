#include "world_render.h"

#include <iostream>
#include <GL/glew.h>
#include <memory>

#include "../graphics/ChunksRenderer.h"
#include "../window/Window.h"
#include "../window/Camera.h"
#include "../graphics/Mesh.h"
#include "../graphics/Shader.h"
#include "../graphics/Texture.h"
#include "../graphics/LineBatch.h"
#include "../graphics/Batch3D.h"
#include "../voxels/Chunks.h"
#include "../voxels/Chunk.h"
#include "../voxels/Block.h"
#include "../world/World.h"
#include "../world/Level.h"
#include "../world/LevelEvents.h"
#include "../objects/Player.h"
#include "../assets/Assets.h"
#include "../objects/player_control.h"

using std::shared_ptr;

WorldRenderer::WorldRenderer(Level* level, Assets* assets) : assets(assets), level(level) {
	lineBatch = new LineBatch(4096);
	batch3d = new Batch3D(1024);
	renderer = new ChunksRenderer(level);
	level->events->listen(EVT_CHUNK_HIDDEN, [this](lvl_event_type type, Chunk* chunk) {
		renderer->unload(chunk);
	});
}

WorldRenderer::~WorldRenderer() {
	delete batch3d;
	delete lineBatch;
	delete renderer;
}

bool WorldRenderer::drawChunk(size_t index, Camera* camera, Shader* shader, bool occlusion){
	shared_ptr<Chunk> chunk = level->chunks->chunks[index];
	if (!chunk->isLighted())
		return false;
	shared_ptr<Mesh> mesh = renderer->getOrRender(chunk.get());
	if (mesh == nullptr)
		return false;

	// Simple frustum culling
	if (occlusion){
		float y = camera->position.y+camera->front.y * CHUNK_H * 0.5f;
		if (y < 0.0f)
			y = 0.0f;
		if (y > CHUNK_H)
			y = CHUNK_H;
		vec3 v = vec3(chunk->x*CHUNK_W, y, chunk->z*CHUNK_D)-camera->position;
		if (v.x*v.x+v.z*v.z > (CHUNK_W*3)*(CHUNK_W*3)) {
			if (dot(camera->front, v) < 0.0f){
				return true;
			}
		}
	}
	mat4 model = glm::translate(mat4(1.0f), vec3(chunk->x*CHUNK_W, 0.0f, chunk->z*CHUNK_D+1));
	shader->uniformMatrix("u_model", model);
	mesh->draw(GL_TRIANGLES);
	return false;
}


void WorldRenderer::draw(Camera* camera, bool occlusion, float fogFactor, float fogCurve){
	Chunks* chunks = level->chunks;

	vec4 skyColor(0.7f, 0.81f, 1.0f, 1.0f);
	glClearColor(skyColor.r, skyColor.g, skyColor.b, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Window::viewport(0, 0, Window::width, Window::height);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	Texture* texture = assets->getTexture("block");
	Shader* shader = assets->getShader("main");
	Shader* linesShader = assets->getShader("lines");
	shader->use();
	shader->uniformMatrix("u_proj", camera->getProjection());
	shader->uniformMatrix("u_view", camera->getView());
	shader->uniform1f("u_gamma", 1.6f);
	shader->uniform3f("u_skyLightColor", 1.1f,1.1f,1.1f);
	shader->uniform3f("u_fogColor", skyColor.r,skyColor.g,skyColor.b);
	shader->uniform1f("u_fogFactor", fogFactor);
	shader->uniform1f("u_fogCurve", fogCurve);
	shader->uniform3f("u_cameraPos", camera->position.x,camera->position.y,camera->position.z);

	Block* cblock = Block::blocks[level->player->choosenBlock];
	shader->uniform3f("u_torchlightColor",
			cblock->emission[0] / 15.0f,
			cblock->emission[1] / 15.0f,
			cblock->emission[2] / 15.0f);
	shader->uniform1f("u_torchlightDistance", 6.0f);
	texture->bind();

	std::vector<size_t> indices;

	for (size_t i = 0; i < chunks->volume; i++){
		shared_ptr<Chunk> chunk = chunks->chunks[i];
		if (chunk == nullptr)
			continue;
		indices.push_back(i);
	}

	float px = camera->position.x / (float)CHUNK_W;
	float pz = camera->position.z / (float)CHUNK_D;
	std::sort(indices.begin(), indices.end(), [this, chunks, px, pz](size_t i, size_t j) {
		shared_ptr<Chunk> a = chunks->chunks[i];
		shared_ptr<Chunk> b = chunks->chunks[j];
		return ((a->x + 0.5f - px)*(a->x + 0.5f - px) + (a->z + 0.5f - pz)*(a->z + 0.5f - pz) >
				(b->x + 0.5f - px)*(b->x + 0.5f - px) + (b->z + 0.5f - pz)*(b->z + 0.5f - pz));
	});


	int occludedChunks = 0;
	for (size_t i = 0; i < indices.size(); i++){
		occludedChunks += drawChunk(indices[i], camera, shader, occlusion);
	}

	shader->uniformMatrix("u_model", mat4(1.0f));
	batch3d->begin();
	// draw 3D stuff here
	batch3d->render();

	if (level->playerController->selectedBlockId != -1){
		Block* selectedBlock = Block::blocks[level->playerController->selectedBlockId];
		vec3 pos = level->playerController->selectedBlockPosition;
		linesShader->use();
		linesShader->uniformMatrix("u_projview", camera->getProjection()*camera->getView());
		glLineWidth(2.0f);
		if (selectedBlock->model == 1){
			lineBatch->box(pos.x+0.5f, pos.y+0.5f, pos.z+0.5f, 1.005f,1.005f,1.005f, 0,0,0,0.5f);
		} else if (selectedBlock->model == 2){
			lineBatch->box(pos.x+0.5f, pos.y+0.35f, pos.z+0.5f, 0.805f,0.705f,0.805f, 0,0,0,0.5f);
		}
		lineBatch->render();
	}

	if (level->player->debug) {
		linesShader->use();
		linesShader->uniformMatrix("u_projview", camera->getProjection()*camera->getView());

		vec3 point = vec3(camera->position.x+camera->front.x,
						  camera->position.y+camera->front.y,
						  camera->position.z+camera->front.z);

		glDisable(GL_DEPTH_TEST);

		glLineWidth(4.0f);
		lineBatch->line(point.x, point.y, point.z,
						point.x+0.1f, point.y, point.z,
						0, 0, 0, 1);
		lineBatch->line(point.x, point.y, point.z,
						point.x, point.y, point.z+0.1f,
						0, 0, 0, 1);
		lineBatch->line(point.x, point.y, point.z,
						point.x, point.y+0.1f, point.z,
						0, 0, 0, 1);
		lineBatch->render();

		glLineWidth(2.0f);
		lineBatch->line(point.x, point.y, point.z,
						point.x+0.1f, point.y, point.z,
						1, 0, 0, 1);
		lineBatch->line(point.x, point.y, point.z,
						point.x, point.y, point.z+0.1f,
						0, 0, 1, 1);
		lineBatch->line(point.x, point.y, point.z,
						point.x, point.y+0.1f, point.z,
						0, 1, 0, 1);
		lineBatch->render();

		glEnable(GL_DEPTH_TEST);
	}
}
