#include "Assets.h"

#include "../graphics/Texture.h"
#include "../graphics/Shader.h"
#include "../graphics/Atlas.h"
#include "../graphics/Font.h"

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

	for (auto& iter : atlases) {
		delete iter.second;
	}
}

Texture* Assets::getTexture(const std::string& name) const {
	auto found = textures.find(name);
	if (found == textures.end())
		return nullptr;
	return found->second;
}

void Assets::store(Texture* texture, const std::string& name){
	textures[name] = texture;
}


Shader* Assets::getShader(const std::string& name) const{
	auto found = shaders.find(name);
	if (found == shaders.end())
		return nullptr;
	return found->second;
}

void Assets::store(Shader* shader, const std::string& name){
	shaders[name] = shader;
}


Font* Assets::getFont(const std::string& name) const {
	auto found = fonts.find(name);
	if (found == fonts.end())
		return nullptr;
	return found->second;
}

void Assets::store(Font* font, const std::string& name){
	fonts[name] = font;
}

Atlas* Assets::getAtlas(const std::string& name) const {
	auto found = atlases.find(name);
	if (found == atlases.end())
		return nullptr;
	return found->second;
}

void Assets::store(Atlas* atlas, const std::string& name){
	atlases[name] = atlas;
}
