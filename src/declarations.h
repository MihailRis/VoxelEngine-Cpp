#ifndef DECLARATIONS_H
#define DECLARATIONS_H

#include <iostream>
#include "Assets.h"
#include "graphics/Shader.h"
#include "graphics/Texture.h"
#include "window/Window.h"

#include "voxels/Block.h"


// Shaders, textures, renderers

bool _load_shader(Assets* assets, std::string vertex_file, std::string fragment_file, std::string name){
	Shader* shader = load_shader(vertex_file, fragment_file);
	if (shader == nullptr){
		std::cerr << "failed to load shader '" << name << "'" << std::endl;
		return false;
	}
	assets->store(shader, name);
	return true;
}

bool _load_texture(Assets* assets, std::string filename, std::string name){
	Texture* texture = load_texture(filename);
	if (texture == nullptr){
		std::cerr << "failed to load texture '" << name << "'" << std::endl;
		return false;
	}
	assets->store(texture, name);
	return true;
}

int initialize_assets(Assets* assets) {
#define LOAD_SHADER(VERTEX, FRAGMENT, NAME) \
	if (!_load_shader(assets, VERTEX, FRAGMENT, NAME))\
		return 1;
#define LOAD_TEXTURE(FILENAME, NAME) \
	if (!_load_texture(assets, FILENAME, NAME))\
		return 1;

	LOAD_SHADER("res/main.glslv", "res/main.glslf", "main");
	LOAD_SHADER("res/crosshair.glslv", "res/crosshair.glslf", "crosshair");
	LOAD_SHADER("res/lines.glslv", "res/lines.glslf", "lines");

	LOAD_TEXTURE("res/block.png", "block");
	return 0;
}


// All in-game definitions (blocks, items, etc..)
void setup_definitions() {
	// AIR
	Block* block = new Block(0,0);
	block->drawGroup = 1;
	block->lightPassing = true;
	block->skyLightPassing = true;
	block->obstacle = false;
	block->selectable = false;
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
	block->selectable = false;
	Block::blocks[block->id] = block;
}
#endif // DECLARATIONS_H

