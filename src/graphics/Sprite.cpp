#include "Sprite.h"

#include "Texture.h"

Sprite::Sprite(glm::vec2 position, glm::vec2 size, Texture* texture)
	: position(position),
	  size(size),
	  origin(0.5f, 0.5f),
	  color(1.0f, 1.0f, 1.0f, 1.0f),
	  angle(0.0f),
	  texture(texture),
	  region() {
}

Sprite::~Sprite() {
}

