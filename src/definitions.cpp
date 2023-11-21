#include "definitions.h"

#include "content/Content.h"
#include "window/Window.h"
#include "window/Events.h"
#include "window/input.h"
#include "voxels/Block.h"

// All in-game definitions (blocks, items, etc..)
void setup_definitions(ContentBuilder* builder) {
	// TODO: automatic atlas generation instead of using texture indices
	Block* block = new Block("core:air", 0);
	block->drawGroup = 1;
	block->lightPassing = true;
	block->skyLightPassing = true;
	block->obstacle = false;
	block->selectable = false;
	block->model = BlockModel::none;
	builder->add(block);

	block = new Block("base:dirt", 2);
	builder->add(block);

	block = new Block("base:grass_block", 4);
	block->textureFaces[2] = 2;
	block->textureFaces[3] = 1;
	builder->add(block);

	block = new Block("base:lamp", 3);
	block->emission[0] = 15;
	block->emission[1] = 14;
	block->emission[2] = 13;
	builder->add(block);

	block = new Block("base:glass",5);
	block->drawGroup = 2;
	block->lightPassing = true;
	builder->add(block);

	block = new Block("base:planks", 6);
	builder->add(block);

	block = new Block("base:wood", 7);
	block->textureFaces[2] = 8;
	block->textureFaces[3] = 8;
	block->rotatable = true;
	builder->add(block);

	block = new Block("base:leaves", 9);
	builder->add(block);

	block = new Block("base:stone", 10);
	builder->add(block);

	block = new Block("base:water", 11);
	block->drawGroup = 4;
	block->lightPassing = true;
	block->skyLightPassing = false;
	block->obstacle = false;
	block->selectable = false;
	builder->add(block);

	block = new Block("base:sand", 12);
	builder->add(block);

	block = new Block("base:bedrock", 13);
	block->breakable = false;
	builder->add(block);

	block = new Block("base:grass", 14);
	block->drawGroup = 5;
	block->lightPassing = true;
	block->obstacle = false;
	block->model = BlockModel::xsprite;
	block->hitboxScale = 0.5f;
	builder->add(block);

	block = new Block("base:flower", 16);
	block->drawGroup = 5;
	block->lightPassing = true;
	block->obstacle = false;
	block->model = BlockModel::xsprite;
	builder->add(block);

	block = new Block("base:brick", 17);
	builder->add(block);

	block = new Block("base:metal", 18);
	builder->add(block);

	block = new Block("base:rust", 19);
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