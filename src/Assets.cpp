#include "Assets.h"

#include "graphics/Texture.h"
#include "graphics/Shader.h"
#include "graphics/Font.h"

Assets::~Assets() {
	for (auto& iter : shaders){
		delete iter.second;
	}

	for (auto& iter : textures){
		delete iter.second;
	}

	for (auto& iter : fonts){
		delete iter.second;
	}
}

Texture* Assets::getTexture(std::string name){
	return textures[name];
}

void Assets::store(Texture* texture, std::string name){
	textures[name] = texture;
}


Shader* Assets::getShader(std::string name){
	return shaders[name];
}

void Assets::store(Shader* shader, std::string name){
	shaders[name] = shader;
}


Font* Assets::getFont(std::string name){
	return fonts[name];
}

void Assets::store(Font* font, std::string name){
	fonts[name] = font;
}
