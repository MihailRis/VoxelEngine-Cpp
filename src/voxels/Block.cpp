#include "Block.h"

BlockRotProfile BlockRotProfile::PIPE {{
		// Vertical
		{{1, 0, 0}, {0, 1, 0}, {0, 0, 1},	{0, 0, 0}},
		// X-Aligned
		{{0, -1, 0}, {1, 0, 0}, {0, 0, 1},	{0, 1, 0}},
		// Z-Aligned
		{{1, 0, 0}, {0, 0, 1}, {0, -1, 0},	{0, 0, -1}},
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
