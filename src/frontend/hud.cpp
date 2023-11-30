#include "hud.h"

#include <iostream>
#include <sstream>
#include <memory>
#include <assert.h>
#include <stdexcept>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "../typedefs.h"
#include "../content/Content.h"
#include "../util/stringutil.h"
#include "../util/timeutil.h"
#include "../assets/Assets.h"
#include "../graphics/Shader.h"
#include "../graphics/Batch2D.h"
#include "../graphics/Font.h"
#include "../graphics/Atlas.h"
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
#include "../maths/voxmaths.h"
#include "gui/controls.h"
#include "gui/panels.h"
#include "gui/UINode.h"
#include "gui/GUI.h"
#include "ContentGfxCache.h"
#include "screens.h"
#include "WorldRenderer.h"
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

HudRenderer::HudRenderer(Engine* engine, 
						 Level* level, 
						 const ContentGfxCache* cache,
						  WorldRenderer* renderer) 
            : level(level), 
			  assets(engine->getAssets()), 
			  gui(engine->getGUI()),
			  cache(cache),
			  renderer(renderer) {
	auto menu = gui->getMenu();
	batch = new Batch2D(1024);
	uicamera = new Camera(vec3(), 1);
	uicamera->perspective = false;
	uicamera->flipped = true;

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
	panel->add(shared_ptr<Label>(create_label([=](){
		auto& settings = engine->getSettings();
		bool culling = settings.graphics.frustumCulling;
		return L"frustum-culling: "+wstring(culling ? L"on" : L"off");
	})));
	panel->add(shared_ptr<Label>(create_label([this, level]() {
		return L"chunks: "+std::to_wstring(this->level->chunks->chunksCount)+
			   L" visible: "+std::to_wstring(level->chunks->visible);
	})));
	panel->add(shared_ptr<Label>(create_label([this](){
		auto player = this->level->player;
		auto indices = this->level->content->indices;
		auto def = indices->getBlockDef(player->selectedVoxel.id);
		std::wstringstream stream;
		stream << std::hex << this->level->player->selectedVoxel.states;
		if (def) {
			stream << L" (" << util::str2wstr_utf8(def->name) << L")";
		}
		return L"block: "+std::to_wstring(player->selectedVoxel.id)+
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
		sub->add(label);
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

		sub->add(box);
		panel->add(sub);
	}
	panel->add(shared_ptr<Label>(create_label([this](){
		int hour, minute, second;
		timeutil::from_value(this->level->world->daytime, hour, minute, second);

		std::wstring timeString = 
					 util::lfill(std::to_wstring(hour), 2, L'0') + L":" +
					 util::lfill(std::to_wstring(minute), 2, L'0');
		return L"time: "+timeString;
	})));
	{
		TrackBar* bar = new TrackBar(0.0f, 1.0f, 1.0f, 0.005f, 8);
		bar->supplier([=]() {
			return level->world->daytime;
		});
		bar->consumer([=](double val) {
			level->world->daytime = val;
		});
		panel->add(bar);
	}
	{
        Panel* checkpanel = new Panel(vec2(400, 32), vec4(5.0f), 1.0f);
        checkpanel->color(vec4(0.0f));
        checkpanel->orientation(Orientation::horizontal);

        CheckBox* checkbox = new CheckBox();
        checkbox->margin(vec4(0.0f, 0.0f, 5.0f, 0.0f));
        checkbox->supplier([=]() {
            return engine->getSettings().debug.showChunkBorders;
        });
        checkbox->consumer([=](bool checked) {
            engine->getSettings().debug.showChunkBorders = checked;
        });
        checkpanel->add(checkbox);
        checkpanel->add(new Label(L"Show Chunk Borders"));

        panel->add(checkpanel);
	}
	panel->refresh();
	menu->reset();
	
	gui->add(this->debugPanel);
}

HudRenderer::~HudRenderer() {
	gui->remove(debugPanel);
	delete batch;
	delete uicamera;
}

void HudRenderer::drawDebug(int fps){
	this->fps = fps;
	fpsMin = min(fps, fpsMin);
	fpsMax = max(fps, fpsMax);
}

/* Inventory temporary replaced with blocks access panel */
void HudRenderer::drawContentAccess(const GfxContext& ctx, Player* player) {
	const Content* content = level->content;
	const ContentIndices* contentIds = content->indices;

	const Viewport& viewport = ctx.getViewport();
	const uint width = viewport.getWidth();
	Atlas* atlas = assets->getAtlas("blocks");

	uint count = contentIds->countBlockDefs();
	uint icon_size = 48;
	uint interval = 4;
	uint inv_cols = 8;
	uint inv_rows = ceildiv(count-1, inv_cols);
	int pad_x = interval;
	int pad_y = interval;
	uint inv_w = inv_cols * icon_size + (inv_cols-1) * interval + pad_x * 2;
	uint inv_h = inv_rows * icon_size + (inv_rows-1) * interval + pad_x * 2;
	int inv_x = (width - (inv_w));
	int inv_y = 0;
	int xs = inv_x + pad_x;
	int ys = inv_y + pad_y;

	vec4 tint = vec4(1.0f);
	int mx = Events::x;
	int my = Events::y;

	// background
	batch->texture(nullptr);
	batch->color = vec4(0.0f, 0.0f, 0.0f, 0.5f);
	batch->rect(inv_x, inv_y, inv_w, inv_h);

	// blocks & items
	batch->texture(atlas->getTexture());
	for (uint i = 0; i < count-1; i++) {
		Block* cblock = contentIds->getBlockDef(i+1);
		if (cblock == nullptr)
			break;
		int x = xs + (icon_size+interval) * (i % inv_cols);
		int y = ys + (icon_size+interval) * (i / inv_cols);
		if (mx > x && mx < x + (int)icon_size && my > y && my < y + (int)icon_size) {
			tint.r *= 1.2f;
			tint.g *= 1.2f;
			tint.b *= 1.2f;
			if (Events::jclicked(mousecode::BUTTON_1)) {
				player->choosenBlock = i+1;
			}
		} else {
			tint = vec4(1.0f);
		}
		drawBlockPreview(cblock, x, y, icon_size, icon_size, tint);
	}
}

void HudRenderer::drawBlockPreview(const Block* def, float x, float y, float w, float h, vec4 tint) {
	if (def->model == BlockModel::block){
		batch->blockSprite(x, y, w, h, &cache->getRegion(def->rt.id, 0), tint);
	} else if (def->model == BlockModel::aabb) {
		batch->blockSprite(x, y, w, h, &cache->getRegion(def->rt.id, 0), tint, def->hitbox.size());
	} else if (def->model == BlockModel::xsprite){
		batch->sprite(x, y, w, h, cache->getRegion(def->rt.id, 3), tint);
	}
}

void HudRenderer::update() {
	auto menu = gui->getMenu();
	if (pause && menu->current().panel == nullptr) {
		pause = false;
	}
	if (Events::jpressed(keycode::ESCAPE) && !gui->isFocusCaught()) {
		if (pause) {
			pause = false;
			menu->reset();
		} else if (inventoryOpen) {
			inventoryOpen = false;
		} else {
			pause = true;
			menu->set("pause");
		}
	}
	if (Events::jactive(BIND_HUD_INVENTORY)) {
		if (!pause) {
			inventoryOpen = !inventoryOpen;
		}
	}
	if ((pause || inventoryOpen) == Events::_cursor_locked) {
		Events::toggleCursor();
	}
}

void HudRenderer::draw(const GfxContext& ctx){
	const Content* content = level->content;
	const ContentIndices* contentIds = content->indices;

	const Viewport& viewport = ctx.getViewport();
	const uint width = viewport.getWidth();
	const uint height = viewport.getHeight();

	Atlas* atlas = assets->getAtlas("blocks");

	debugPanel->visible(level->player->debug);

	uicamera->fov = height;

	Shader* uishader = assets->getShader("ui");
	uishader->use();
	uishader->uniformMatrix("u_projview", uicamera->getProjection()*uicamera->getView());

	batch->begin();

	// Chosen block preview
	batch->color = vec4(1.0f);
	if (Events::_cursor_locked && !level->player->debug) {
		batch->lineWidth(2);
		batch->line(width/2, height/2-6, width/2, height/2+6, 0.2f, 0.2f, 0.2f, 1.0f);
		batch->line(width/2+6, height/2, width/2-6, height/2, 0.2f, 0.2f, 0.2f, 1.0f);
		batch->line(width/2-5, height/2-5, width/2+5, height/2+5, 0.9f, 0.9f, 0.9f, 1.0f);
		batch->line(width/2+5, height/2-5, width/2-5, height/2+5, 0.9f, 0.9f, 0.9f, 1.0f);
	}
	Player* player = level->player;


	batch->color = vec4(0.0f, 0.0f, 0.0f, 0.5f);
	batch->rect(width - 68, height - 68, 68, 68);

	batch->color = vec4(1.0f);
	batch->texture(atlas->getTexture());
	{
		Block* cblock = contentIds->getBlockDef(player->choosenBlock);
		assert(cblock != nullptr);
		drawBlockPreview(cblock, width - 56, uicamera->fov - 56, 48, 48, vec4(1.0f));
	}

	if (pause) {
		batch->texture(nullptr);
		batch->color = vec4(0.0f, 0.0f, 0.0f, 0.5f);
		batch->rect(0, 0, width, height);
	}
	if (inventoryOpen) {
        drawContentAccess(ctx, player);
	}
	batch->render();
}

bool HudRenderer::isInventoryOpen() const {
	return inventoryOpen;
}

bool HudRenderer::isPause() const {
	return pause;
}