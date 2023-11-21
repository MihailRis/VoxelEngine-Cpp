#include "definitions.h"

#include "content/Content.h"
#include "window/Window.h"
#include "window/Events.h"
#include "window/input.h"
#include "voxels/Block.h"

// All in-game definitions (blocks, items, etc..)
void setup_definitions(ContentBuilder* builder) {
	// TODO: automatic atlas generation instead of using texture indices
	Block* block = new Block("core:air", "air");
	block->drawGroup = 1;
	block->lightPassing = true;
	block->skyLightPassing = true;
	block->obstacle = false;
	block->selectable = false;
	block->model = BlockModel::none;
	builder->add(block);

	block = new Block("base:dirt", "dirt");
	builder->add(block);

	block = new Block("base:grass_block", "grass_side");
	block->textureFaces[2] = "dirt";
	block->textureFaces[3] = "grass_top";
	builder->add(block);

	block = new Block("base:lamp", "lamp");
	block->emission[0] = 15;
	block->emission[1] = 14;
	block->emission[2] = 13;
	builder->add(block);

	block = new Block("base:glass", "glass");
	block->drawGroup = 2;
	block->lightPassing = true;
	builder->add(block);

	block = new Block("base:planks", "planks");
	builder->add(block);

	block = new Block("base:wood", "wood");
	block->textureFaces[2] = "wood_top";
	block->textureFaces[3] = "wood_top";
	block->rotatable = true;
	builder->add(block);

	block = new Block("base:leaves", "leaves");
	builder->add(block);

	block = new Block("base:stone", "stone");
	builder->add(block);

	block = new Block("base:water", "water");
	block->drawGroup = 4;
	block->lightPassing = true;
	block->skyLightPassing = false;
	block->obstacle = false;
	block->selectable = false;
	builder->add(block);

	block = new Block("base:sand", "sand");
	builder->add(block);

	block = new Block("base:bedrock", "bedrock");
	block->breakable = false;
	builder->add(block);

	block = new Block("base:grass", "grass");
	block->drawGroup = 5;
	block->lightPassing = true;
	block->obstacle = false;
	block->model = BlockModel::xsprite;
	block->hitboxScale = 0.5f;
	builder->add(block);

	block = new Block("base:flower", "flower");
	block->drawGroup = 5;
	block->lightPassing = true;
	block->obstacle = false;
	block->model = BlockModel::xsprite;
	builder->add(block);

	block = new Block("base:brick", "brick");
	builder->add(block);

	block = new Block("base:metal", "metal");
	builder->add(block);

	block = new Block("base:rust", "rust");
	builder->add(block);
}

void setup_bindings() {
	Events::bind(BIND_MOVE_FORWARD, inputtype::keyboard, keycode::W);
	Events::bind(BIND_MOVE_BACK, inputtype::keyboard, keycode::S);
	Events::bind(BIND_MOVE_RIGHT, inputtype::keyboard, keycode::D);
	Events::bind(BIND_MOVE_LEFT, inputtype::keyboard, keycode::A);
	Events::bind(BIND_MOVE_JUMP, inputtype::keyboard, keycode::SPACE);
	Events::bind(BIND_MOVE_SPRINT, inputtype::keyboard, keycode::LEFT_CONTROL);
	Events::bind(BIND_MOVE_CROUCH, inputtype::keyboard, keycode::LEFT_SHIFT);
	Events::bind(BIND_MOVE_CHEAT, inputtype::keyboard, keycode::R);
	Events::bind(BIND_CAM_ZOOM, inputtype::keyboard, keycode::C);
	Events::bind(BIND_PLAYER_NOCLIP, inputtype::keyboard, keycode::N);
	Events::bind(BIND_PLAYER_FLIGHT, inputtype::keyboard, keycode::F);
	Events::bind(BIND_HUD_INVENTORY, inputtype::keyboard, keycode::TAB);
}