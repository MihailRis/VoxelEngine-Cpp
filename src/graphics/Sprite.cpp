#include "Sprite.h"

#include "../graphics-common/ITexture.h"

Sprite::Sprite(glm::vec2 position, glm::vec2 size, ITexture* texture)
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

