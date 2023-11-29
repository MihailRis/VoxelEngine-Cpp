#include "world_render.h"

#include <iostream>
#include <GL/glew.h>
#include <memory>
#include <assert.h>

#include "../content/Content.h"
#include "../graphics/ChunksRenderer.h"
#include "../window/Window.h"
#include "../window/Camera.h"
#include "../graphics/Mesh.h"
#include "../graphics/Atlas.h"
#include "../graphics/Shader.h"
#include "../graphics/Texture.h"
#include "../graphics/LineBatch.h"
#include "../voxels/Chunks.h"
#include "../voxels/Chunk.h"
#include "../voxels/Block.h"
#include "../world/World.h"
#include "../world/Level.h"
#include "../world/LevelEvents.h"
#include "../objects/Player.h"
#include "../assets/Assets.h"
#include "../objects/player_control.h"
#include "../maths/FrustumCulling.h"
#include "../maths/voxmaths.h"
#include "../settings.h"
#include "../engine.h"
#include "ContentGfxCache.h"
#include "graphics/Skybox.h"

using glm::vec3;
using std::string;
using std::shared_ptr;

WorldRenderer::WorldRenderer(Engine* engine, Level* level, const ContentGfxCache* cache) 
	: engine(engine), level(level) {
	EngineSettings& settings = engine->getSettings();

	lineBatch = new LineBatch(4096);
	renderer = new ChunksRenderer(level, cache, settings);
	frustumCulling = new Frustum();
	level->events->listen(EVT_CHUNK_HIDDEN, [this](lvl_event_type type, Chunk* chunk) {
		renderer->unload(chunk);
	});
	skybox = new Skybox(64, engine->getAssets()->getShader("skybox_gen"));
}

WorldRenderer::~WorldRenderer() {
	delete skybox;
	delete lineBatch;
	delete renderer;
	delete frustumCulling;
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
		vec3 min(chunk->x * CHUNK_W, chunk->bottom, chunk->z * CHUNK_D);
		vec3 max(chunk->x * CHUNK_W + CHUNK_W, chunk->top, chunk->z * CHUNK_D + CHUNK_D);

		if (!frustumCulling->IsBoxVisible(min, max)) return false;
	}
	mat4 model = glm::translate(mat4(1.0f), vec3(chunk->x*CHUNK_W, 0.0f, chunk->z*CHUNK_D+1));
	shader->uniformMatrix("u_model", model);
	mesh->draw();
	return true;
}

void WorldRenderer::drawChunks(Chunks* chunks, 
							   Camera* camera, 
							   Shader* shader, 
							   bool occlusion) {
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

	if (occlusion) frustumCulling->update(camera->getProjView());
	chunks->visible = 0;
	for (size_t i = 0; i < indices.size(); i++){
		chunks->visible += drawChunk(indices[i], camera, shader, occlusion);
	}
}


void WorldRenderer::draw(const GfxContext& pctx, Camera* camera, bool occlusion){
	EngineSettings& settings = engine->getSettings();
	skybox->refresh(level->world->daytime, 
					fmax(1.0f, 18.0f/settings.chunks.loadDistance), 4);

	const Content* content = level->content;
	const ContentIndices* contentIds = content->indices;
	Assets* assets = engine->getAssets();
	Atlas* atlas = assets->getAtlas("blocks");
	Shader* shader = assets->getShader("main");
	Shader* linesShader = assets->getShader("lines");

	const Viewport& viewport = pctx.getViewport();
	int displayWidth = viewport.getWidth();
	int displayHeight = viewport.getHeight();
	Window::clearDepth();
	Window::viewport(0, 0, displayWidth, displayHeight);

	Shader* backShader = assets->getShader("background");
	backShader->use();
	backShader->uniformMatrix("u_view", camera->getView(false));
	backShader->uniform1f("u_zoom", camera->zoom);
	backShader->uniform1f("u_ar", (float)Window::width/(float)Window::height);
	skybox->draw(backShader);

	{
		GfxContext ctx = pctx.sub();
		ctx.depthTest(true);
		ctx.cullFace(true);

		float fogFactor = 18.0f / (float)settings.chunks.loadDistance;

		shader->use();
		skybox->bind();
		shader->uniformMatrix("u_proj", camera->getProjection());
		shader->uniformMatrix("u_view", camera->getView());
		shader->uniform1f("u_gamma", 1.0f);
		shader->uniform1f("u_fogFactor", fogFactor);
		shader->uniform1f("u_fogCurve", settings.graphics.fogCurve);
		shader->uniform3f("u_cameraPos", camera->position);
		shader->uniform1i("u_cubemap", 1);

		Block* cblock = contentIds->getBlockDef(level->player->choosenBlock);
		assert(cblock != nullptr);
		float multiplier = 0.5f;
		shader->uniform3f("u_torchlightColor",
				cblock->emission[0] / 15.0f * multiplier,
				cblock->emission[1] / 15.0f * multiplier,
				cblock->emission[2] / 15.0f * multiplier);
		shader->uniform1f("u_torchlightDistance", 6.0f);
		atlas->getTexture()->bind();

		Chunks* chunks = level->chunks;
		drawChunks(chunks, camera, shader, occlusion);

		shader->uniformMatrix("u_model", mat4(1.0f));

		if (level->playerController->selectedBlockId != -1){
			Block* block = contentIds->getBlockDef(level->playerController->selectedBlockId);
			assert(block != nullptr);
			vec3 pos = level->playerController->selectedBlockPosition;
			linesShader->use();
			linesShader->uniformMatrix("u_projview", camera->getProjView());
			lineBatch->lineWidth(2.0f);
			float size = block->hitboxScale;
			lineBatch->box(pos.x+0.5f, pos.y+size*0.5f, pos.z+0.5f, 
						   size, size, size, 0, 0, 0, 0.5f);
			lineBatch->render();
		}
		skybox->unbind();
	}

	if (level->player->debug) {
		GfxContext ctx = pctx.sub();
		ctx.depthTest(true);

		linesShader->use();
		if (settings.debug.showChunkBorders){
			linesShader->uniformMatrix("u_projview", camera->getProjView());
			vec3 coord = level->player->camera->position;
			if (coord.x < 0) coord.x--;
			if (coord.z < 0) coord.z--;
			int cx = floordiv((int)coord.x, CHUNK_W);
			int cz = floordiv((int)coord.z, CHUNK_D);
			/*corner*/ {
				lineBatch->line( cx    * CHUNK_W, 0,        cz    * CHUNK_D,     
								 cx    * CHUNK_W, CHUNK_H,  cz    * CHUNK_D,     0.8f, 0, 0.8f, 1);
				lineBatch->line( cx    * CHUNK_W, 0,       (cz+1) * CHUNK_D,     
								 cx    * CHUNK_W, CHUNK_H, (cz+1) * CHUNK_D,     0.8f, 0, 0.8f, 1);
				lineBatch->line((cx+1) * CHUNK_W, 0,        cz    * CHUNK_D,     
								(cx+1) * CHUNK_W, CHUNK_H,  cz    * CHUNK_D,     0.8f, 0, 0.8f, 1);
				lineBatch->line((cx+1) * CHUNK_W, 0,       (cz+1) * CHUNK_D,     
								(cx+1) * CHUNK_W, CHUNK_H, (cz+1) * CHUNK_D,     0.8f, 0, 0.8f, 1);
			}
			for (int i = 2; i < CHUNK_W; i+=2) {
				lineBatch->line( cx    * CHUNK_W + i, 0,        cz    * CHUNK_D,     
								 cx    * CHUNK_W + i, CHUNK_H,  cz    * CHUNK_D,     0, 0, 0.8f, 1);
				lineBatch->line( cx    * CHUNK_W + i, 0,       (cz+1) * CHUNK_D,     
								 cx    * CHUNK_W + i, CHUNK_H, (cz+1) * CHUNK_D,     0, 0, 0.8f, 1);
			}
			for (int i = 2; i < CHUNK_D; i+=2) {
				lineBatch->line( cx    * CHUNK_W,     0,        cz    * CHUNK_D + i, 
								 cx    * CHUNK_W,     CHUNK_H,  cz    * CHUNK_D + i, 0.8f, 0, 0, 1);
				lineBatch->line((cx+1) * CHUNK_W,     0,        cz    * CHUNK_D + i, 
								(cx+1) * CHUNK_W,     CHUNK_H,  cz    * CHUNK_D + i, 0.8f, 0, 0, 1);
			}
			for (int i=0; i < CHUNK_H; i+=2){
				lineBatch->line( cx    * CHUNK_W, i,  cz    * CHUNK_D,
								 cx    * CHUNK_W, i, (cz+1) * CHUNK_D, 0, 0.8f, 0, 1);
				lineBatch->line( cx    * CHUNK_W, i, (cz+1) * CHUNK_D,
								(cx+1) * CHUNK_W, i, (cz+1) * CHUNK_D, 0, 0.8f, 0, 1);
				lineBatch->line((cx+1) * CHUNK_W, i, (cz+1) * CHUNK_D,
								(cx+1) * CHUNK_W, i,  cz    * CHUNK_D, 0, 0.8f, 0, 1);
				lineBatch->line((cx+1) * CHUNK_W, i,  cz    * CHUNK_D,
								 cx    * CHUNK_W, i,  cz    * CHUNK_D, 0, 0.8f, 0, 1);

			}
			lineBatch->render();
		}

		float length = 40.f;
		// top-right: vec3 tsl = vec3(displayWidth - length - 4, -length - 4, 0.f);
		vec3 tsl = vec3(displayWidth/2, displayHeight/2, 0.f);
		glm::mat4 model(glm::translate(glm::mat4(1.f), tsl));
		linesShader->uniformMatrix("u_projview", glm::ortho(
				0.f, (float)displayWidth, 
				0.f, (float)displayHeight,
				-length, length) * model * glm::inverse(camera->rotation));

		ctx.depthTest(false);
		lineBatch->lineWidth(4.0f);
		lineBatch->line(0.f, 0.f, 0.f, length, 0.f, 0.f, 0.f, 0.f, 0.f, 1.f);
		lineBatch->line(0.f, 0.f, 0.f, 0.f, length, 0.f, 0.f, 0.f, 0.f, 1.f);
		lineBatch->line(0.f, 0.f, 0.f, 0.f, 0.f, length, 0.f, 0.f, 0.f, 1.f);
		lineBatch->render();

		ctx.depthTest(true);
		lineBatch->lineWidth(2.0f);
		lineBatch->line(0.f, 0.f, 0.f, length, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f);
		lineBatch->line(0.f, 0.f, 0.f, 0.f, length, 0.f, 0.f, 1.f, 0.f, 1.f);
		lineBatch->line(0.f, 0.f, 0.f, 0.f, 0.f, length, 0.f, 0.f, 1.f, 1.f);
		lineBatch->render();
	}
}
