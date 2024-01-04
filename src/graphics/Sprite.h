#ifndef SRC_GRAPHICS_SPRITE_H_
#define SRC_GRAPHICS_SPRITE_H_

#include <glm/glm.hpp>
#include "UVRegion.h"

class ITexture;

class Sprite {
public:
	glm::vec2 position;
	glm::vec2 size;
	glm::vec2 origin;
	glm::vec4 color;
	float angle;
	bool flippedX = false;
	bool flippedY = false;
	ITexture* texture;
	UVRegion region;

	Sprite(glm::vec2 position, glm::vec2 size, ITexture* texture);
	virtual ~Sprite();

	void setTexture(ITexture* texture) {
		this->texture = texture;
	}
};

#endif /* SRC_GRAPHICS_SPRITE_H_ */
