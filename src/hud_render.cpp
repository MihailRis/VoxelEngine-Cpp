#include "hud_render.h"

#include <GL/glew.h>
#include "Assets.h"
#include "graphics/Shader.h"
#include "graphics/Batch2D.h"
#include "graphics/Font.h"
#include "window/Camera.h"
#include "window/Window.h"
#include "voxels/Chunks.h"
#include "voxels/Block.h"
#include "world/World.h"
#include "world/Level.h"
#include "objects/Player.h"


HudRenderer::HudRenderer() {
	batch = new Batch2D(1024);
	uicamera = new Camera(glm::vec3(), Window::height / 1.0f);
	uicamera->perspective = false;
	uicamera->flipped = true;
}

HudRenderer::~HudRenderer() {
	delete batch;
	delete uicamera;
}


void HudRenderer::draw(World* world, Level* level, Assets* assets, bool devdata, int fps){
	Chunks* chunks = level->chunks;
	Player* player = level->player;

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	Shader* uishader = assets->getShader("ui");
	uishader->use();
	uishader->uniformMatrix("u_projview", uicamera->getProjection()*uicamera->getView());

	// Draw debug info
	Font* font = assets->getFont("normal");
	batch->begin();
	if (devdata){
		font->draw(batch, L"chunks: "+std::to_wstring(chunks->chunksCount), 16, 16, STYLE_OUTLINE);
		font->draw(batch, std::to_wstring((int)player->camera->position.x), 10, 30, STYLE_OUTLINE);
		font->draw(batch, std::to_wstring((int)player->camera->position.y), 50, 30, STYLE_OUTLINE);
		font->draw(batch, std::to_wstring((int)player->camera->position.z), 90, 30, STYLE_OUTLINE);
		font->draw(batch, L"fps:", 16, 42, STYLE_OUTLINE);
		font->draw(batch, std::to_wstring(fps), 40, 42, STYLE_OUTLINE);
	}
	batch->render();

	// Chosen block preview
	Texture* blocks = assets->getTexture("block");
	Texture* sprite = assets->getTexture("sprite");

	batch->texture(sprite);
	batch->sprite(16, 640, 64, 64, 16, 0, vec4(1.0f));

	batch->texture(blocks);
	Block* cblock = Block::blocks[player->choosenBlock];
	if (cblock->model == BLOCK_MODEL_CUBE){
		batch->blockSprite(24, 648, 48, 48, 16, cblock->textureFaces, vec4(1.0f));
	} else if (cblock->model == BLOCK_MODEL_GRASS){
		batch->sprite(24, 648, 48, 48, 16, cblock->textureFaces[3], vec4(1.0f));
	}

	batch->render();
}
