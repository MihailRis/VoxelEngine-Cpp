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
#include "../graphics/Batch3D.h"
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
#include "BlocksPreview.h"
#include "InventoryView.h"
#include "LevelFrontend.h"
#include "../engine.h"
#include "../core_defs.h"
#include "../items/ItemDef.h"

using glm::vec2;
using glm::vec3;
using glm::vec4;
using namespace gui;

inline std::shared_ptr<Label> create_label(gui::wstringsupplier supplier) {
	auto label = std::make_shared<Label>(L"-");
	label->textSupplier(supplier);
	return label;
}

void HudRenderer::createDebugPanel(Engine* engine) {
    auto level = frontend->getLevel();

    Panel* panel = new Panel(vec2(250, 200), vec4(5.0f), 1.0f);
	debugPanel = std::shared_ptr<UINode>(panel);
	panel->listenInterval(1.0f, [this]() {
		fpsString = std::to_wstring(fpsMax)+L" / "+std::to_wstring(fpsMin);
		fpsMin = fps;
		fpsMax = fps;
	});
	panel->setCoord(vec2(10, 10));
	panel->add(create_label([this](){ return L"fps: "+this->fpsString;}));
	panel->add(create_label([this](){
		return L"meshes: " + std::to_wstring(Mesh::meshesCount);
	}));
	panel->add(create_label([=](){
		auto& settings = engine->getSettings();
		bool culling = settings.graphics.frustumCulling;
		return L"frustum-culling: "+std::wstring(culling ? L"on" : L"off");
	}));
	panel->add(create_label([=]() {
		return L"chunks: "+std::to_wstring(level->chunks->chunksCount)+
			   L" visible: "+std::to_wstring(level->chunks->visible);
	}));
	panel->add(create_label([=](){
		auto player = level->player;
		auto indices = level->content->indices;
		auto def = indices->getBlockDef(player->selectedVoxel.id);
		std::wstringstream stream;
		stream << std::hex << level->player->selectedVoxel.states;
		if (def) {
			stream << L" (" << util::str2wstr_utf8(def->name) << L")";
		}
		return L"block: "+std::to_wstring(player->selectedVoxel.id)+
		       L" "+stream.str();
	}));
	panel->add(create_label([=](){
		return L"seed: "+std::to_wstring(level->world->seed);
	}));

	for (int ax = 0; ax < 3; ax++){
		Panel* sub = new Panel(vec2(10, 27), vec4(0.0f));
		sub->orientation(Orientation::horizontal);

		std::wstring str = L"x: ";
		str[0] += ax;
		Label* label = new Label(str);
		label->margin(vec4(2, 3, 2, 3));
		sub->add(label);
		sub->color(vec4(0.0f));

		// Coord input
		TextBox* box = new TextBox(L"");
		box->textSupplier([=]() {
			Hitbox* hitbox = level->player->hitbox.get();
			return std::to_wstring(hitbox->position[ax]);
		});
		box->textConsumer([=](std::wstring text) {
			try {
				vec3 position = level->player->hitbox->position;
				position[ax] = std::stoi(text);
				level->player->teleport(position);
			} catch (std::invalid_argument& _){
			}
		});

		sub->add(box);
		panel->add(sub);
	}
	panel->add(create_label([=](){
		int hour, minute, second;
		timeutil::from_value(level->world->daytime, hour, minute, second);

		std::wstring timeString = 
					 util::lfill(std::to_wstring(hour), 2, L'0') + L":" +
					 util::lfill(std::to_wstring(minute), 2, L'0');
		return L"time: "+timeString;
	}));
	{
		TrackBar* bar = new TrackBar(0.0f, 1.0f, 1.0f, 0.005f, 8);
		bar->supplier([=]() {return level->world->daytime;});
		bar->consumer([=](double val) {level->world->daytime = val;});
		panel->add(bar);
	}
	{
		TrackBar* bar = new TrackBar(0.0f, 1.0f, 0.0f, 0.005f, 8);
		bar->supplier([=]() {return WorldRenderer::fog;});
		bar->consumer([=](double val) {WorldRenderer::fog = val;});
		panel->add(bar);
	}
	{
        auto checkbox = new FullCheckBox(L"Show Chunk Borders", vec2(400, 32));
        checkbox->supplier([=]() {
            return engine->getSettings().debug.showChunkBorders;
        });
        checkbox->consumer([=](bool checked) {
            engine->getSettings().debug.showChunkBorders = checked;
        });
        panel->add(checkbox);
	}
	panel->refresh();
}

HudRenderer::HudRenderer(Engine* engine, LevelFrontend* frontend) 
    : assets(engine->getAssets()), 
      gui(engine->getGUI()),
      frontend(frontend) {

    auto level = frontend->getLevel();
	auto menu = gui->getMenu();
    auto content = level->content;
    auto indices = content->indices;

    std::vector<itemid_t> items;
    for (itemid_t id = 1; id < indices->countItemDefs(); id++) {
        items.push_back(id);
    }
    contentAccess.reset(new InventoryView(8, content, frontend, items));
    contentAccess->setSlotConsumer([=](blockid_t id) {
        level->player->setChosenItem(id);
    });

    hotbarView.reset(new InventoryView(1, content, frontend, std::vector<itemid_t> {0}));

	uicamera = new Camera(vec3(), 1);
	uicamera->perspective = false;
	uicamera->flipped = true;

    createDebugPanel(engine);
	menu->reset();
	
	gui->add(this->debugPanel);
}

HudRenderer::~HudRenderer() {
	gui->remove(debugPanel);
	delete uicamera;
}

void HudRenderer::drawDebug(int fps){
	this->fps = fps;
	fpsMin = min(fps, fpsMin);
	fpsMax = max(fps, fpsMax);
}

void HudRenderer::update(bool visible) {
	auto menu = gui->getMenu();
    if (!visible && inventoryOpen) {
        inventoryOpen = false;
    }
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
	if (visible && Events::jactive(BIND_HUD_INVENTORY)) {
		if (!pause) {
			inventoryOpen = !inventoryOpen;
		}
	}
	if ((pause || inventoryOpen) == Events::_cursor_locked) {
		Events::toggleCursor();
	}
}

void HudRenderer::drawOverlay(const GfxContext& ctx) {
	if (pause) {
	    Shader* uishader = assets->getShader("ui");
	    uishader->use();
	    uishader->uniformMatrix("u_projview", uicamera->getProjection()*uicamera->getView());

        const Viewport& viewport = ctx.getViewport();
        const uint width = viewport.getWidth();
        const uint height = viewport.getHeight();
        auto batch = ctx.getBatch2D();
        batch->begin();

		// draw fullscreen dark overlay
		batch->texture(nullptr);
		batch->color = vec4(0.0f, 0.0f, 0.0f, 0.5f);
		batch->rect(0, 0, width, height);
        batch->render();
	}
} 

void HudRenderer::draw(const GfxContext& ctx){
    auto level = frontend->getLevel();

	const Viewport& viewport = ctx.getViewport();
	const uint width = viewport.getWidth();
	const uint height = viewport.getHeight();

	Player* player = level->player;
	debugPanel->visible(player->debug);

	uicamera->setFov(height);

    auto batch = ctx.getBatch2D();
	batch->begin();

	Shader* uishader = assets->getShader("ui");
	uishader->use();
	uishader->uniformMatrix("u_projview", uicamera->getProjection()*uicamera->getView());
	
	// Draw selected item preview
    hotbarView->setPosition(width-60, height-60);
    hotbarView->setItems({player->getChosenItem()});
    hotbarView->actAndDraw(&ctx);

	// Crosshair
	batch->begin();
	if (!pause && Events::_cursor_locked && !level->player->debug) {
		batch->lineWidth(2);
		batch->line(width/2, height/2-6, width/2, height/2+6, 0.2f, 0.2f, 0.2f, 1.0f);
		batch->line(width/2+6, height/2, width/2-6, height/2, 0.2f, 0.2f, 0.2f, 1.0f);
		batch->line(width/2-5, height/2-5, width/2+5, height/2+5, 0.9f, 0.9f, 0.9f, 1.0f);
		batch->line(width/2+5, height/2-5, width/2-5, height/2+5, 0.9f, 0.9f, 0.9f, 1.0f);
	}

	if (inventoryOpen) {
		// draw content access panel (all available items)
        contentAccess->setPosition(viewport.getWidth()-contentAccess->getWidth(), 0);
        contentAccess->actAndDraw(&ctx);
	}
	batch->render();
}

bool HudRenderer::isInventoryOpen() const {
	return inventoryOpen;
}

bool HudRenderer::isPause() const {
	return pause;
}
