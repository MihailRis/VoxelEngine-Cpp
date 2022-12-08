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
	// float vertices[] = {
	// 		// x    y
	// 	   -0.01f,-0.01f,
	// 	    0.01f, 0.01f,

	// 	   -0.01f, 0.01f,
	// 	    0.01f,-0.01f,
	// };
	// int attrs[] = {
	// 		2,  0 //null terminator
	// };
	// crosshair = new Mesh(vertices, 4, attrs);

	batch = new Batch2D(1024);
	uicamera = new Camera(glm::vec3(), Window::height / 1.0f);
	uicamera->perspective = false;
	uicamera->flipped = true;
}

HudRenderer::~HudRenderer() {
	// delete crosshair;
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
	font->draw(batch, std::to_wstring((int)player->camera->position.y), 90, 30, STYLE_OUTLINE);
	font->draw(batch, std::to_wstring((int)player->camera->position.z), 170, 30, STYLE_OUTLINE);
	font->draw(batch, L"fps:", 16, 42, STYLE_OUTLINE);
	font->draw(batch, std::to_wstring(fps), 44, 42, STYLE_OUTLINE);
	font->draw(batch, L"occlusion: "+std::to_wstring(occlusion), 16, 54, STYLE_OUTLINE);
	// batch->render();
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

	batch->texture(nullptr);
	batch->color = vec4(1.0f);
	if (Events::_cursor_locked && !level->player->debug) {
		glLineWidth(2);
		batch->line(Window::width/2, Window::height/2-6, Window::width/2, Window::height/2+6, 0.2f, 0.2f, 0.2f, 1.0f);
		batch->line(Window::width/2+6, Window::height/2, Window::width/2-6, Window::height/2, 0.2f, 0.2f, 0.2f, 1.0f);
		batch->line(Window::width/2-5, Window::height/2-5, Window::width/2+5, Window::height/2+5, 0.9f, 0.9f, 0.9f, 1.0f);
		batch->line(Window::width/2+5, Window::height/2-5, Window::width/2-5, Window::height/2+5, 0.9f, 0.9f, 0.9f, 1.0f);
	}

	// batch->texture(sprite);
	// batch->sprite(Window::width/2-32, uicamera->fov - 80, 64, 64, 16, 0, vec4(1.0f));
	// batch->rect(Window::width/2-128-4, Window::height-80-4, 256+8, 64+8,
	// 					0.85f, 0.85f, 0.85f, 0.95f, 0.95f, 0.95f,
	// 					0.55f, 0.55f, 0.55f,
	// 					0.45f, 0.45f, 0.45f, 0.7f, 0.7f, 0.7f, 2);
	batch->rect(Window::width/2-128-4, Window::height-80-4, 256+8, 64+8,
						0.95f, 0.95f, 0.95f, 0.85f, 0.85f, 0.85f,
						0.7f, 0.7f, 0.7f,
						0.55f, 0.55f, 0.55f, 0.45f, 0.45f, 0.45f, 4);
	batch->rect(Window::width/2-128, Window::height - 80, 256, 64,
						0.75f, 0.75f, 0.75f, 0.75f, 0.75f, 0.75f,
						0.75f, 0.75f, 0.75f,
						0.75f, 0.75f, 0.75f, 0.75f, 0.75f, 0.75f, 4);
	batch->rect(Window::width/2-32+2, Window::height - 80+2, 60, 60,
						0.45f, 0.45f, 0.45f, 0.55f, 0.55f, 0.55f,
						0.7f, 0.7f, 0.7f,
						0.85f, 0.85f, 0.85f, 0.95f, 0.95f, 0.95f, 2);
	batch->rect(Window::width/2-32+4, Window::height - 80+4, 56, 56,
						0.75f, 0.75f, 0.75f, 0.75f, 0.75f, 0.75f,
						0.75f, 0.75f, 0.75f,
						0.75f, 0.75f, 0.75f, 0.75f, 0.75f, 0.75f, 2);


	batch->texture(blocks);
	Player* player = level->player;
	{
		Block* cblock = Block::blocks[player->choosenBlock];
		if (cblock->model == BLOCK_MODEL_CUBE){
			batch->blockSprite(Window::width/2-24, uicamera->fov - 72, 48, 48, 16, cblock->textureFaces, vec4(1.0f));
		} else if (cblock->model == BLOCK_MODEL_X_SPRITE){
			batch->sprite(Window::width/2-24, uicamera->fov - 72, 48, 48, 16, cblock->textureFaces[3], vec4(1.0f));
		}
	}

	if (!Events::_cursor_locked) { //inventory
		int size = 48;
		int step = 64;
		int inv_wm = step*10;
		int inv_hm = step*8;
		int inv_w = inv_wm - (step - size);
		int inv_h = inv_hm - (step - size);
		int inv_x = (Window::width - (inv_w)) / 2;
		int inv_y = (Window::height - (inv_h)) / 2;
		int xs = (Window::width - inv_w + step)/2;
		int ys = (Window::height - inv_h + step)/2;
		if (Window::width > inv_w*3){
			inv_x = (Window::width + (inv_w)) / 2;
			inv_y = (Window::height - (inv_h)) / 2;
			xs = (Window::width + inv_w + step)/2;
			ys = (Window::height - inv_h + step)/2;
		}
		int x = 0;
		int y = 0;
		vec4 tint = vec4(1.0f);
		int mx = Events::x;
		int my = Events::y;
		int count = (inv_w / step) * (inv_h / step) + 1;

		//back
		batch->texture(nullptr);
		batch->color = vec4(0.0f, 0.0f, 0.0f, 0.3f);
		batch->rect(0, 0, Window::width, Window::height);
		batch->rect(inv_x - 4, inv_y - 4, inv_w+8, inv_h+8,
						0.95f, 0.95f, 0.95f, 0.85f, 0.85f, 0.85f,
						0.7f, 0.7f, 0.7f,
						0.55f, 0.55f, 0.55f, 0.45f, 0.45f, 0.45f, 4);
		batch->rect(inv_x, inv_y, inv_w, inv_h,
						0.75f, 0.75f, 0.75f, 0.75f, 0.75f, 0.75f,
						0.75f, 0.75f, 0.75f,
						0.75f, 0.75f, 0.75f, 0.75f, 0.75f, 0.75f, 4);

		batch->color = vec4(0.35f, 0.35f, 0.35f, 1.0f);
		for (unsigned i = 1; i < count; i++) {
			x = xs + step * ((i-1) % (inv_w / step));
			y = ys + step * ((i-1) / (inv_w / step));
			// batch->rect(x-2, y-2, size+4, size+4);
			batch->rect(x-2, y-2, size+4, size+4,
						0.45f, 0.45f, 0.45f, 0.55f, 0.55f, 0.55f,
						0.7f, 0.7f, 0.7f,
						0.85f, 0.85f, 0.85f, 0.95f, 0.95f, 0.95f, 2);
			batch->rect(x, y, size, size,
						0.65f, 0.65f, 0.65f, 0.65f, 0.65f, 0.65f,
						0.65f, 0.65f, 0.65f,
						0.65f, 0.65f, 0.65f, 0.65f, 0.65f, 0.65f, 2);
		}

		// batch->color = vec4(0.5f, 0.5f, 0.5f, 1.0f);
		// for (unsigned i = 1; i < count; i++) {
		// 	x = xs + step * ((i-1) % (inv_w / step));
		// 	y = ys + step * ((i-1) / (inv_w / step));
		// 	batch->rect(x, y, size, size);
		// }

		//front
		batch->texture(blocks);
		for (unsigned i = 1; i < count; i++) {
			Block* cblock = Block::blocks[i];
			if (cblock == nullptr)
				break;
			x = xs + step * ((i-1) % (inv_w / step));
			y = ys + step * ((i-1) / (inv_w / step));
			if (mx > x && mx < x + size && my > y && my < y + size) {
				tint.r *= 1.2f;
				tint.g *= 1.2f;
				tint.b *= 1.2f;
				if (Events::jclicked(GLFW_MOUSE_BUTTON_LEFT)) {
					player->choosenBlock = i;
				}
				// size = 50;
			} else
			{
				// size = 48;
				tint = vec4(1.0f);
			}
			
			if (cblock->model == BLOCK_MODEL_CUBE){
				batch->blockSprite(x, y, size, size, 16, cblock->textureFaces, tint);
			} else if (cblock->model == BLOCK_MODEL_X_SPRITE){
				batch->sprite(x, y, size, size, 16, cblock->textureFaces[3], tint);
			}
		}
	}

	// batch->render();

	if (Events::_cursor_locked && !level->player->debug){
		// Shader* crosshairShader = assets->getShader("crosshair");
		// crosshairShader->use();
		// crosshairShader->uniform1f("u_ar", (float)Window::height / (float)Window::width);
		// crosshairShader->uniform1f("u_scale", 1.0f / ((float)Window::height / 1000.0f));
		// glLineWidth(2.0f);
		// crosshair->draw(GL_LINES);
	}
}
