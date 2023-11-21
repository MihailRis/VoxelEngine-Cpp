#include "Block.h"

Block::Block(std::string name, int texture) : name(name),
		textureFaces{texture,texture,texture,texture,texture,texture},
		emission{0,0,0}{
}
