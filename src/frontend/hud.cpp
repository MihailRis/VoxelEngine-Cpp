#include "hud.h"

#include <iostream>
#include <sstream>
#include <memory>
#include <stdexcept>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "../typedefs.h"
#include "../util/stringutil.h"
#include "../assets/Assets.h"
#include "../graphics/Shader.h"
#include "../graphics/Batch2D.h"
#include "../graphics/Font.h"
#include "../graphics/Mesh.h"
#include "../window/Camera.h"
#include "../window/Window.h"
#include "../window/Events.h"
#include "../window/input.h"
#include "../voxels/Chunks.h"
#include "../voxels/Block.h"
#include "../world/World.h"
#include "../world/Level.h"
#include "../objects/Player.h"
#include "../physics/Hitbox.h"
#include "gui/controls.h"
#include "gui/panels.h"
#include "gui/UINode.h"
#include "gui/GUI.h"

using std::wstring;
using std::shared_ptr;
using glm::vec2;
using glm::vec3;
using glm::vec4;
using namespace gui;

inline Label* create_label(gui::wstringsupplier supplier) {
	Label* label = new Label(L"-");
	label->textSupplier(supplier);
	return label;
}

HudRenderer::HudRenderer(GUI* gui, Level* level, Assets* assets) : level(level), assets(assets), guiController(gui) {
	batch = new Batch2D(1024);
	uicamera = new Camera(vec3(), Window::height);
	uicamera->perspective = false;
	uicamera->flipped = true;

	gui->interval(1.0f, [this]() {
		fpsString = std::to_wstring(fpsMax)+L" / "+std::to_wstring(fpsMin);
		fpsMin = fps;
		fpsMax = fps;
	});
	Panel* panel = new Panel(vec2(200, 200), vec4(5.0f), 1.0f);
	panel->setCoord(vec2(10, 10));
	panel->add(shared_ptr<Label>(create_label([this](){
		return L"chunks: "+std::to_wstring(this->level->chunks->chunksCount);
	})));
	panel->add(shared_ptr<Label>(create_label([this](){
		return L"fps: "+this->fpsString;
	})));
	panel->add(shared_ptr<Label>(create_label([this](){
		return L"occlusion: "+wstring(this->occlusion ? L"on" : L"off");
	})));
	panel->add(shared_ptr<Label>(create_label([this](){
		std::wstringstream stream;
		stream << std::hex << this->level->player->selectedVoxel.states;
		return L"block-selected: "+std::to_wstring(this->level->player->selectedVoxel.id)+L" "+stream.str();
	})));
	panel->add(shared_ptr<Label>(create_label([this](){
		return L"meshes: " + std::to_wstring(Mesh::meshesCount);
	})));
	for (int ax = 0; ax < 3; ax++){
		Panel* sub = new Panel(vec2(10, 27), vec4(0.0f));
		sub->orientation(Orientation::horizontal);

		wstring str = L"x: ";
		str[0] += ax;
		Label* label = new Label(str);
		label->margin(vec4(2, 3, 2, 3));
		sub->add(shared_ptr<UINode>(label));
		sub->color(vec4(0.0f));

		// Coordinate input
		TextBox* box = new TextBox(L"");
		box->textSupplier([this, ax]() {
			Hitbox* hitbox = this->level->player->hitbox;
			return std::to_wstring((int)hitbox->position[ax]);
		});
		box->textConsumer([this, ax](wstring text) {
			try {
				vec3 position = this->level->player->hitbox->position;
				position[ax] = std::stoi(text);
				this->level->player->teleport(position);
			} catch (std::invalid_argument& _){
			}
		});

		sub->add(shared_ptr<UINode>(box));
		panel->add(shared_ptr<UINode>(sub));
	}
	panel->refresh();
	debugPanel = panel;

	pauseMenu = new Panel(vec2(350, 200));
	pauseMenu->color(vec4(0.0f));
	{
		Button* button = new Button(L"Continue", vec4(12.0f, 10.0f, 12.0f, 12.0f));
		button->listenAction([this](GUI*){
			this->pause = false;
		});
		pauseMenu->add(shared_ptr<UINode>(button));
	}
	{
		Button* button = new Button(L"Save and Quit", vec4(12.0f, 10.0f, 12.0f, 12.0f));
		button->listenAction([this](GUI*){
			Window::setShouldClose(true);
		});
		pauseMenu->add(shared_ptr<UINode>(button));
	}
	guiController->add(shared_ptr<UINode>(debugPanel));
	guiController->add(shared_ptr<UINode>(pauseMenu));
}

HudRenderer::~HudRenderer() {
	delete batch;
	delete uicamera;
}

void HudRenderer::drawDebug(int fps, bool occlusion){
	this->occlusion = occlusion;
	this->fps = fps;
	fpsMin = min(fps, fpsMin);
	fpsMax = max(fps, fpsMax);
}

void HudRenderer::drawInventory(Player* player) {
	Texture* blocks = assets->getTexture("block_tex");
	uint size = 48;
	uint step = 64;
	uint inv_cols = 10;
	uint inv_rows = 8;
	uint inv_w = step*inv_cols + size;
	uint inv_h = step*inv_rows + size;
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
	vec4 tint = vec4(1.0f);
	int mx = Events::x;
	int my = Events::y;
	uint count = inv_cols * inv_rows;

	// back
	batch->texture(nullptr);
	batch->color = vec4(0.0f, 0.0f, 0.0f, 0.3f);
	batch->rect(inv_x - 4, inv_y - 4, inv_w+8, inv_h+8,
					0.95f, 0.95f, 0.95f, 0.85f, 0.85f, 0.85f,
					0.7f, 0.7f, 0.7f,
					0.55f, 0.55f, 0.55f, 0.45f, 0.45f, 0.45f, 4);
	batch->rect(inv_x, inv_y, inv_w, inv_h,
					0.75f, 0.75f, 0.75f, 0.75f, 0.75f, 0.75f,
					0.75f, 0.75f, 0.75f,
					0.75f, 0.75f, 0.75f, 0.75f, 0.75f, 0.75f, 4);

	batch->color = vec4(0.35f, 0.35f, 0.35f, 1.0f);
	for (uint i = 0; i < count; i++) {
		int x = xs + step * (i % (inv_cols));
		int y = ys + step * (i / (inv_cols));
		batch->rect(x-2, y-2, size+4, size+4,
					0.45f, 0.45f, 0.45f, 0.55f, 0.55f, 0.55f,
					0.7f, 0.7f, 0.7f,
					0.85f, 0.85f, 0.85f, 0.95f, 0.95f, 0.95f, 2);
		batch->rect(x, y, size, size,
					0.65f, 0.65f, 0.65f, 0.65f, 0.65f, 0.65f,
					0.65f, 0.65f, 0.65f,
					0.65f, 0.65f, 0.65f, 0.65f, 0.65f, 0.65f, 2);
	}

	// front
	batch->texture(blocks);
	for (uint i = 0; i < count; i++) {
		Block* cblock = Block::blocks[i+1];
		if (cblock == nullptr)
			break;
		int x = xs + step * (i % inv_cols);
		int y = ys + step * (i / inv_cols);
		if (mx > x && mx < x + (int)size && my > y && my < y + (int)size) {
			tint.r *= 1.2f;
			tint.g *= 1.2f;
			tint.b *= 1.2f;
			if (Events::jclicked(GLFW_MOUSE_BUTTON_LEFT)) {
				player->choosenBlock = i+1;
			}
		} else
		{
			tint = vec4(1.0f);
		}
		
		if (cblock->model == BlockModel::block){
			batch->blockSprite(x, y, size, size, 16, cblock->textureFaces, tint);
		} else if (cblock->model == BlockModel::xsprite){
			batch->sprite(x, y, size, size, 16, cblock->textureFaces[3], tint);
		}
	}
}

void HudRenderer::draw(){
	debugPanel->visible(level->player->debug);
	pauseMenu->visible(pause);
	pauseMenu->setCoord(vec2(Window::width/2.0f, Window::height/2.0f) - pauseMenu->size() / 2.0f);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	uicamera->fov = Window::height;

	Shader* uishader = assets->getShader("ui");
	uishader->use();
	uishader->uniformMatrix("u_projview", uicamera->getProjection()*uicamera->getView());

	// Chosen block preview
	Texture* blocks = assets->getTexture("block_tex");

	batch->texture(nullptr);
	batch->color = vec4(1.0f);
	if (Events::_cursor_locked && !level->player->debug) {
		glLineWidth(2);
		const uint width = Window::width;
		const uint height = Window::height;
		batch->line(width/2, height/2-6, width/2, height/2+6, 0.2f, 0.2f, 0.2f, 1.0f);
		batch->line(width/2+6, height/2, width/2-6, height/2, 0.2f, 0.2f, 0.2f, 1.0f);
		batch->line(width/2-5, height/2-5, width/2+5, height/2+5, 0.9f, 0.9f, 0.9f, 1.0f);
		batch->line(width/2+5, height/2-5, width/2-5, height/2+5, 0.9f, 0.9f, 0.9f, 1.0f);
	}
	Player* player = level->player;

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

	{
		Block* cblock = Block::blocks[player->choosenBlock];
		if (cblock->model == BlockModel::block){
			batch->blockSprite(Window::width/2-24, uicamera->fov - 72, 48, 48, 16, cblock->textureFaces, vec4(1.0f));
		} else if (cblock->model == BlockModel::xsprite){
			batch->sprite(Window::width/2-24, uicamera->fov - 72, 48, 48, 16, cblock->textureFaces[3], vec4(1.0f));
		}
	}

	if (Events::jpressed(keycode::ESCAPE) && !guiController->isFocusCaught()) {
		if (pause) {
			pause = false;
		} else if (inventoryOpen) {
			inventoryOpen = false;
		} else {
			pause = true;
		}
	}
	if (Events::jpressed(keycode::TAB)) {
		if (!pause) {
			inventoryOpen = !inventoryOpen;
		}
	}
	if ((pause || inventoryOpen) == Events::_cursor_locked)
		Events::toggleCursor();

	if (pause || inventoryOpen) {
		batch->texture(nullptr);
		batch->color = vec4(0.0f, 0.0f, 0.0f, 0.5f);
		batch->rect(0, 0, Window::width, Window::height);
	}
	if (inventoryOpen) {
        drawInventory(player);
	}
	batch->render();
}

bool HudRenderer::isInventoryOpen() const {
	return inventoryOpen;
}

bool HudRenderer::isPause() const {
	return pause;
}