#include "Block.h"

BlockRotProfile BlockRotProfile::PIPE {{
		{ { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 }, { 0, 0, 0 } }, // BLOCK_DIR_PY
		{ { 0,-1, 0 }, { 1, 0, 0 }, { 0, 0, 1 }, { 0, 1, 0 } }, // BLOCK_DIR_PX
		{ { 1, 0, 0 }, { 0, 0, 1 }, { 0,-1, 0 }, { 0, 0,-1 } }, // BLOCK_DIR_PZ
		{ { 0, 1, 0 }, {-1, 0, 0 }, { 0, 0, 1 }, { 1, 0, 0 } }, // BLOCK_DIR_MX
		{ {-1, 0, 0 }, { 0,-1, 0 }, { 0, 0, 1 }, { 1, 1, 0 } }, // BLOCK_DIR_MY
		{ { 1, 0, 0 }, { 0, 0,-1 }, { 0, 1, 0 }, { 0, 1, 0 } }, // BLOCK_DIR_MZ
}};

Block::Block(std::string name) 
	: name(name), 
	  textureFaces {"notfound","notfound","notfound",
	  			    "notfound","notfound","notfound",} {
	rotations = BlockRotProfile::PIPE;
}

Block::Block(std::string name, std::string texture) : name(name),
		textureFaces{texture,texture,texture,texture,texture,texture} {
}
