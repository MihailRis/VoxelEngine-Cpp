#include "Block.h"

Block::Block(std::string name) 
	: name(name), 
	  textureFaces({"notfound","notfound","notfound",
	  			    "notfound","notfound","notfound",}) {

}

Block::Block(std::string name, std::string texture) : name(name),
		textureFaces{texture,texture,texture,texture,texture,texture} {
}
