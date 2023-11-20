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
#include "screens.h"
#include "../engine.h"
#include "../core_defs.h"

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

HudRenderer::HudRenderer(Engine* engine, Level* level) : level(level), assets(engine->getAssets()), gui(engine->getGUI()) {
	batch = new Batch2D(1024);
	uicamera = new Camera(vec3(), 1);
	uicamera->perspective = false;
	uicamera->flipped = true;

	auto pagesptr = gui->get("pages");
	PagesControl* pages = (PagesControl*)(pagesptr.get());

	Panel* panel = new Panel(vec2(250, 200), vec4(5.0f), 1.0f);
	debugPanel = shared_ptr<UINode>(panel);
	panel->listenInterval(1.0f, [this]() {
		fpsString = std::to_wstring(fpsMax)+L" / "+std::to_wstring(fpsMin);
		fpsMin = fps;
		fpsMax = fps;
	});
	panel->setCoord(vec2(10, 10));
	panel->add(shared_ptr<Label>(create_label([this](){
		return L"fps: "+this->fpsString;
	})));
	panel->add(shared_ptr<Label>(create_label([this](){
		return L"meshes: " + std::to_wstring(Mesh::meshesCount);
	})));
	panel->add(shared_ptr<Label>(create_label([this](){
		return L"occlusion: "+wstring(this->occlusion ? L"on" : L"off");
	})));
	panel->add(shared_ptr<Label>(create_label([this, level]() {
		return L"chunks: "+std::to_wstring(this->level->chunks->chunksCount)+
			   L" visible: "+std::to_wstring(level->chunks->visible);
	})));
	panel->add(shared_ptr<Label>(create_label([this](){
		std::wstringstream stream;
		stream << std::hex << this->level->player->selectedVoxel.states;

		auto player = this->level->player;
		return L"block-selected: "+std::to_wstring(player->selectedVoxel.id)+
		       L" "+stream.str();
	})));
	panel->add(shared_ptr<Label>(create_label([this](){
		return L"seed: "+std::to_wstring(this->level->world->seed);
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

		// Coord input
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

	panel = new Panel(vec2(350, 200));
	auto pauseMenu = shared_ptr<UINode>(panel);
	panel->color(vec4(0.0f));
	{
		Button* button = new Button(L"Continue", vec4(10.0f));
		button->listenAction([=](GUI*){
			pages->reset();
			pause = false;
		});
		panel->add(shared_ptr<UINode>(button));
	}
	panel->add((new Button(L"Settings", vec4(10.f)))->listenAction([=](GUI* gui) {
        pages->set("settings");
    }));
	{
		Button* button = new Button(L"Save and Quit to Menu", vec4(10.f));
		button->listenAction([this, engine](GUI*){
			engine->setScreen(shared_ptr<Screen>(new MenuScreen(engine)));
		});
		panel->add(shared_ptr<UINode>(button));
	}

	pages->reset();
	pages->add("pause", pauseMenu);
	gui->add(this->debugPanel);
}

HudRenderer::~HudRenderer() {
	gui->remove(debugPanel);
	//gui->remove(gui->get("pages"));
	delete batch;
	delete uicamera;
}

void HudRenderer::drawDebug(int fps, bool occlusion){
	this->occlusion = occlusion;
	this->fps = fps;
	fpsMin = min(fps, fpsMin);
	fpsMax = max(fps, fpsMax);
}

void HudRenderer::drawInventory(const GfxContext& ctx, Player* player) {
	const Viewport& viewport = ctx.getViewport();
	const uint width = viewport.getWidth();
	const uint height = viewport.getHeight();

	Texture* blocks = assets->getTexture("block_tex");
	uint size = 48;
	uint step = 64;
	uint inv_cols = 10;
	uint inv_rows = 8;
	uint inv_w = step*inv_cols + size;
	uint inv_h = step*inv_rows + size;
	int inv_x = (width - (inv_w)) / 2;
	int inv_y = (height - (inv_h)) / 2;
	int xs = (width - inv_w + step)/2;
	int ys = (height - inv_h + step)/2;
	if (width > inv_w*3){
		inv_x = (width + (inv_w)) / 2;
		inv_y = (height - (inv_h)) / 2;
		xs = (width + inv_w + step)/2;
		ys = (height - inv_h + step)/2;
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
		} else {
			tint = vec4(1.0f);
		}
		
		if (cblock->model == BlockModel::block){
			batch->blockSprite(x, y, size, size, 16, cblock->textureFaces, tint);
		} else if (cblock->model == BlockModel::xsprite){
			batch->sprite(x, y, size, size, 16, cblock->textureFaces[3], tint);
		}
	}
}

void HudRenderer::update() {
	PagesControl* pages = (PagesControl*)(gui->get("pages").get());
	if (Events::jpressed(keycode::ESCAPE) && !gui->isFocusCaught()) {
		if (pause) {
			pause = false;
			pages->reset();
		} else if (inventoryOpen) {
			inventoryOpen = false;
		} else {
			pause = true;
			pages->set("pause");
		}
	}
	if (Events::jactive(BIND_HUD_INVENTORY)) {
		if (!pause) {
			inventoryOpen = !inventoryOpen;
		}
	}
	if ((pause || inventoryOpen) == Events::_cursor_locked)
		Events::toggleCursor();
}

void HudRenderer::draw(const GfxContext& ctx){
	const Viewport& viewport = ctx.getViewport();
	const uint width = viewport.getWidth();
	const uint height = viewport.getHeight();
	auto pages = gui->get("pages");

	debugPanel->visible(level->player->debug);
	pages->setCoord((viewport.size() - pages->size()) / 2.0f);

	uicamera->fov = height;

	Shader* uishader = assets->getShader("ui");
	uishader->use();
	uishader->uniformMatrix("u_projview", uicamera->getProjection()*uicamera->getView());

	// Chosen block preview
	Texture* blocks = assets->getTexture("block_tex");

	batch->texture(nullptr);
	batch->color = vec4(1.0f);
	if (Events::_cursor_locked && !level->player->debug) {
		batch->lineWidth(2);
		batch->line(width/2, height/2-6, width/2, height/2+6, 0.2f, 0.2f, 0.2f, 1.0f);
		batch->line(width/2+6, height/2, width/2-6, height/2, 0.2f, 0.2f, 0.2f, 1.0f);
		batch->line(width/2-5, height/2-5, width/2+5, height/2+5, 0.9f, 0.9f, 0.9f, 1.0f);
		batch->line(width/2+5, height/2-5, width/2-5, height/2+5, 0.9f, 0.9f, 0.9f, 1.0f);
	}
	Player* player = level->player;

	batch->rect(width/2-128-4, height-80-4, 256+8, 64+8,
						0.95f, 0.95f, 0.95f, 0.85f, 0.85f, 0.85f,
						0.7f, 0.7f, 0.7f,
						0.55f, 0.55f, 0.55f, 0.45f, 0.45f, 0.45f, 4);
	batch->rect(width/2-128, height - 80, 256, 64,
						0.75f, 0.75f, 0.75f, 0.75f, 0.75f, 0.75f,
						0.75f, 0.75f, 0.75f,
						0.75f, 0.75f, 0.75f, 0.75f, 0.75f, 0.75f, 4);
	batch->rect(width/2-32+2, height - 80+2, 60, 60,
						0.45f, 0.45f, 0.45f, 0.55f, 0.55f, 0.55f,
						0.7f, 0.7f, 0.7f,
						0.85f, 0.85f, 0.85f, 0.95f, 0.95f, 0.95f, 2);
	batch->rect(width/2-32+4, height - 80+4, 56, 56,
						0.75f, 0.75f, 0.75f, 0.75f, 0.75f, 0.75f,
						0.75f, 0.75f, 0.75f,
						0.75f, 0.75f, 0.75f, 0.75f, 0.75f, 0.75f, 2);

	batch->texture(blocks);

	{
		Block* cblock = Block::blocks[player->choosenBlock];
		if (cblock->model == BlockModel::block){
			batch->blockSprite(width/2-24, uicamera->fov - 72, 48, 48, 16, cblock->textureFaces, vec4(1.0f));
		} else if (cblock->model == BlockModel::xsprite){
			batch->sprite(width/2-24, uicamera->fov - 72, 48, 48, 16, cblock->textureFaces[3], vec4(1.0f));
		}
	}

	if (pause || inventoryOpen) {
		batch->texture(nullptr);
		batch->color = vec4(0.0f, 0.0f, 0.0f, 0.5f);
		batch->rect(0, 0, width, height);
	}
	if (inventoryOpen) {
        drawInventory(ctx, player);
	}
	batch->render();
}

bool HudRenderer::isInventoryOpen() const {
	return inventoryOpen;
}

bool HudRenderer::isPause() const {
	return pause;
}