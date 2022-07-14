#ifndef DECLARATIONS_H
#define DECLARATIONS_H

#include <iostream>
#include "Assets.h"
#include "graphics/Shader.h"
#include "graphics/Texture.h"
#include "graphics/Font.h"
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

bool _load_font(Assets* assets, std::string filename, std::string name){
	std::vector<Texture*> pages;
	for (size_t i = 0; i <= 4; i++){
		Texture* texture = load_texture(filename+"_"+std::to_string(i)+".png");
		if (texture == nullptr){
			std::cerr << "failed to load bitmap font '" << name << "' (missing page " << std::to_string(i) << ")" << std::endl;
			return false;
		}
		pages.push_back(texture);
	}
	Font* font = new Font(pages);
	assets->store(font, name);
	return true;
}

int initialize_assets(Assets* assets) {
#define LOAD_SHADER(VERTEX, FRAGMENT, NAME) \
	if (!_load_shader(assets, VERTEX, FRAGMENT, NAME))\
		return 1;
#define LOAD_TEXTURE(FILENAME, NAME) \
	if (!_load_texture(assets, FILENAME, NAME))\
		return 1;
#define LOAD_FONT(FILENAME, NAME) \
	if (!_load_font(assets, FILENAME, NAME))\
		return 1;

	LOAD_SHADER("res/main.glslv", "res/main.glslf", "main");
	LOAD_SHADER("res/crosshair.glslv", "res/crosshair.glslf", "crosshair");
	LOAD_SHADER("res/lines.glslv", "res/lines.glslf", "lines");
	LOAD_SHADER("res/ui.glslv", "res/ui.glslf", "ui");

	LOAD_TEXTURE("res/block.png", "block");

	LOAD_FONT("res/font", "normal");
	return 0;
}


// All in-game definitions (blocks, items, etc..)
void setup_definitions() {
	for (size_t i = 0; i < 256; i++)
		Block::blocks[i] = nullptr;
	// AIR 0
	Block* block = new Block(0,0);
	block->drawGroup = 1;
	block->lightPassing = true;
	block->skyLightPassing = true;
	block->obstacle = false;
	block->selectable = false;
	Block::blocks[block->id] = block;

	// DIRT 1
	block = new Block(1,2);
	Block::blocks[block->id] = block;

	// GRASS 2
	block = new Block(2,4);
	block->textureFaces[2] = 2;
	block->textureFaces[3] = 1;
	Block::blocks[block->id] = block;

	// LAMP 3
	block = new Block(3,3);
	block->emission[0] = 15;
	block->emission[1] = 14;
	block->emission[2] = 13;
	Block::blocks[block->id] = block;

	// GLASS 4
	block = new Block(4,5);
	block->drawGroup = 2;
	block->lightPassing = true;
	Block::blocks[block->id] = block;

	// PLANKS 5
	block = new Block(5,6);
	Block::blocks[block->id] = block;

	// WOOD 6
	block = new Block(6,7);
	block->textureFaces[2] = 8;
	block->textureFaces[3] = 8;
	Block::blocks[block->id] = block;

	// LEAVES 7
	block = new Block(7,9);
	Block::blocks[block->id] = block;

	// STONE 8
	block = new Block(8,10);
	Block::blocks[block->id] = block;

	// WATER 9
	block = new Block(9,11);
	block->drawGroup = 4;
	block->lightPassing = true;
	block->skyLightPassing = false;
	block->obstacle = false;
	block->selectable = false;
	Block::blocks[block->id] = block;

	// SAND 10
	block = new Block(10,12);
	Block::blocks[block->id] = block;

	// BEDROCK 11
	block = new Block(11,13);
	block->breakable = false;
	Block::blocks[block->id] = block;
}
#endif // DECLARATIONS_H

