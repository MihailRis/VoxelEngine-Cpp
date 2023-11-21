#include "Block.h"

Block::Block(std::string name, std::string texture) : name(name),
		textureFaces{texture,texture,texture,texture,texture,texture},
		emission{0,0,0}{
}
