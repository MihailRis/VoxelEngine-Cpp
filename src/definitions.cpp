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
	block->replaceable = true;
	block->drawGroup = 1;
	block->lightPassing = true;
	block->skyLightPassing = true;
	block->obstacle = false;
	block->selectable = false;
	block->model = BlockModel::none;
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