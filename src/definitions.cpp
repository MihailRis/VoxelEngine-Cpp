#include "definitions.h"

#include <glm/glm.hpp>

#include "content/Content.h"
#include "window/Window.h"
#include "window/Events.h"
#include "window/input.h"
#include "voxels/Block.h"

using glm::vec3;

// All in-game definitions (blocks, items, etc..)
void setup_definitions(ContentBuilder* builder) {
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
	block->hitbox.scale(vec3(0.7f), vec3(0.5f, 0.0f, 0.5f));
	builder->add(block);

	block = new Block("base:flower", "flower");
	block->drawGroup = 5;
	block->lightPassing = true;
	block->obstacle = false;
	block->model = BlockModel::xsprite;
	block->hitbox.scale(vec3(0.7f));
	builder->add(block);

	block = new Block("base:brick", "brick");
	builder->add(block);

	block = new Block("base:metal", "metal");
	builder->add(block);

	block = new Block("base:rust", "rust");
	builder->add(block);

	block = new Block("base:red_lamp", "red_lamp");
	block->emission[0] = 15;
	builder->add(block);

	block = new Block("base:green_lamp", "green_lamp");
	block->emission[1] = 15;
	builder->add(block);

	block = new Block("base:blue_lamp", "blue_lamp");
	block->emission[2] = 15;
	builder->add(block);

	// block added for test
	block = new Block("base:pane", "pane");
	block->textureFaces[FACE_MX] = "pane_side";
	block->textureFaces[FACE_PX] = "pane_side";
	block->textureFaces[FACE_MY] = "pane_side_2";
	block->textureFaces[FACE_PY] = "pane_side_2";
	block->model = BlockModel::aabb;
	block->hitbox.scale(vec3(1.0f, 1.0f, 0.2f), vec3(0.5f, 0.5f, 0.0f));
	block->lightPassing = true;
	block->skyLightPassing = true;
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
	Events::bind(BIND_PLAYER_ATTACK, inputtype::mouse, mousecode::BUTTON_1);
	Events::bind(BIND_PLAYER_BUILD, inputtype::mouse, mousecode::BUTTON_2);
	Events::bind(BIND_PLAYER_PICK, inputtype::mouse, mousecode::BUTTON_3);
	Events::bind(BIND_HUD_INVENTORY, inputtype::keyboard, keycode::TAB);
}