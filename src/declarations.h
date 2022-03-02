#ifndef DECLARATIONS_CPP
#define DECLARATIONS_CPP

#include <iostream>

#include "graphics/Shader.h"
#include "graphics/Texture.h"
#include "window/Window.h"

#include "voxels/Block.h"


Shader *shader, *linesShader, *crosshairShader;
Texture *texture;


// Shaders, textures, renderers
int initialize_assets() {
	shader = load_shader("res/main.glslv", "res/main.glslf");
	if (shader == nullptr){
		std::cerr << "failed to load shader" << std::endl;
		Window::terminate();
		return 1;
	}

	crosshairShader = load_shader("res/crosshair.glslv", "res/crosshair.glslf");
	if (crosshairShader == nullptr){
		std::cerr << "failed to load crosshair shader" << std::endl;
		Window::terminate();
		return 1;
	}

	linesShader = load_shader("res/lines.glslv", "res/lines.glslf");
	if (linesShader == nullptr){
		std::cerr << "failed to load lines shader" << std::endl;
		Window::terminate();
		return 1;
	}

	texture = load_texture("res/block.png");
	if (texture == nullptr){
		std::cerr << "failed to load texture" << std::endl;
		delete shader;
		Window::terminate();
		return 1;
	}
	return 0;
}

// Deleting GL objects like shaders, textures
void finalize_assets(){
	delete shader;
	delete texture;
	delete crosshairShader;
	delete linesShader;
}


// All in-game definitions (blocks, items, etc..)
void setup_definitions() {
	// AIR
	Block* block = new Block(0,0);
	block->drawGroup = 1;
	block->lightPassing = true;
	block->skyLightPassing = true;
	block->obstacle = false;
	Block::blocks[block->id] = block;

	// STONE
	block = new Block(1,2);
	Block::blocks[block->id] = block;

	// GRASS
	block = new Block(2,4);
	block->textureFaces[2] = 2;
	block->textureFaces[3] = 1;
	Block::blocks[block->id] = block;

	// LAMP
	block = new Block(3,3);
	block->emission[0] = 15;
	block->emission[1] = 14;
	block->emission[2] = 13;
	Block::blocks[block->id] = block;

	// GLASS
	block = new Block(4,5);
	block->drawGroup = 2;
	block->lightPassing = true;
	Block::blocks[block->id] = block;

	// PLANKS
	block = new Block(5,6);
	Block::blocks[block->id] = block;

	// WOOD
	block = new Block(6,7);
	block->textureFaces[2] = 8;
	block->textureFaces[3] = 8;
	Block::blocks[block->id] = block;

	// LEAVES
	block = new Block(7,9);
	Block::blocks[block->id] = block;

	// ACTUAL STONE
	block = new Block(8,10);
	Block::blocks[block->id] = block;

	// WATER
	block = new Block(9,11);
	block->drawGroup = 4;
	block->lightPassing = true;
	block->skyLightPassing = false;
	block->obstacle = false;
	Block::blocks[block->id] = block;
}
#endif // DECLARATIONS_CPP

