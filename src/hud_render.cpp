#include "hud_render.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Assets.h"
#include "graphics/Shader.h"
#include "graphics/Batch2D.h"
#include "graphics/Font.h"
#include "graphics/Mesh.h"
#include "window/Camera.h"
#include "window/Window.h"
#include "window/Events.h"
#include "voxels/Chunks.h"
#include "voxels/Block.h"
#include "world/World.h"
#include "world/Level.h"
#include "objects/Player.h"


HudRenderer::HudRenderer() {
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
	crosshair = new Mesh(vertices, 4, attrs);

	batch = new Batch2D(1024);
	uicamera = new Camera(glm::vec3(), Window::height / 1.0f);
	uicamera->perspective = false;
	uicamera->flipped = true;
}

HudRenderer::~HudRenderer() {
	delete crosshair;
	delete batch;
	delete uicamera;
}

void HudRenderer::drawDebug(Level* level, Assets* assets, int fps, bool occlusion){
	Chunks* chunks = level->chunks;
	Player* player = level->player;

	Font* font = assets->getFont("normal");

	Shader* uishader = assets->getShader("ui");
	uishader->use();
	uishader->uniformMatrix("u_projview", uicamera->getProjection()*uicamera->getView());
	batch->color = vec4(1.0f);
	batch->begin();
	font->draw(batch, L"chunks: "+std::to_wstring(chunks->chunksCount), 16, 16, STYLE_OUTLINE);
	font->draw(batch, std::to_wstring((int)player->camera->position.x), 10, 30, STYLE_OUTLINE);
	font->draw(batch, std::to_wstring((int)player->camera->position.y), 50, 30, STYLE_OUTLINE);
	font->draw(batch, std::to_wstring((int)player->camera->position.z), 90, 30, STYLE_OUTLINE);
	font->draw(batch, L"fps:", 16, 42, STYLE_OUTLINE);
	font->draw(batch, std::to_wstring(fps), 44, 42, STYLE_OUTLINE);
	font->draw(batch, L"occlusion: "+std::to_wstring(occlusion), 16, 54, STYLE_OUTLINE);
	batch->render();
}


void HudRenderer::draw(Level* level, Assets* assets){
	uicamera->fov = Window::height;

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	Shader* uishader = assets->getShader("ui");
	uishader->use();
	uishader->uniformMatrix("u_projview", uicamera->getProjection()*uicamera->getView());

	// Chosen block preview
	Texture* blocks = assets->getTexture("block");
	Texture* sprite = assets->getTexture("slot");

	if (!Events::_cursor_locked) {
		batch->texture(nullptr);
		batch->color = vec4(0.0f, 0.0f, 0.0f, 0.5f);
		batch->rect(0, 0, Window::width, Window::height);
	}

	batch->color = vec4(1.0f);
	batch->texture(sprite);
	batch->sprite(16, uicamera->fov - 80, 64, 64, 16, 0, vec4(1.0f));

	batch->texture(blocks);
	Player* player = level->player;
	{
		Block* cblock = Block::blocks[player->choosenBlock];
		if (cblock->model == BLOCK_MODEL_CUBE){
			batch->blockSprite(24, uicamera->fov - 72, 48, 48, 16, cblock->textureFaces, vec4(1.0f));
		} else if (cblock->model == BLOCK_MODEL_X_SPRITE){
			batch->sprite(24, uicamera->fov - 72, 48, 48, 16, cblock->textureFaces[3], vec4(1.0f));
		}
	}

	if (!Events::_cursor_locked) {
		int size = 48;
		int step = 70;
		int y = uicamera->fov - 72 - 70;
		int x = 0;
		vec4 tint = vec4(1.0f);
		int mx = Events::x;
		int my = Events::y;

		for (unsigned i = 1; i < 256; i++) {
			Block* cblock = Block::blocks[i];
			if (cblock == nullptr)
				break;
			x = 24 + (i-1) * step;
			y -= 72 * (x / (Window::width - step));
			x %= (Window::width - step);
			if (mx > x && mx < x + size && my > y && my < y + size) {
				tint.r *= 1.3f;
				tint.g *= 1.3f;
				tint.b *= 1.3f;
				if (Events::jclicked(GLFW_MOUSE_BUTTON_LEFT)) {
					player->choosenBlock = i;
				}
			} else
			{
				tint = vec4(1.0f);
			}
			
			if (cblock->model == BLOCK_MODEL_CUBE){
				batch->blockSprite(x, y, size, size, 16, cblock->textureFaces, tint);
			} else if (cblock->model == BLOCK_MODEL_X_SPRITE){
				batch->sprite(x, y, size, size, 16, cblock->textureFaces[3], tint);
			}
		}
	}

	batch->render();

	Shader* crosshairShader = assets->getShader("crosshair");
	crosshairShader->use();
	crosshairShader->uniform1f("u_ar", (float)Window::height / (float)Window::width);
	crosshairShader->uniform1f("u_scale", 1.0f / ((float)Window::height / 1000.0f));
	crosshair->draw(GL_LINES);
}
