#include "Assets.h"

#include "../audio/audio.hpp"
#include "../graphics/core/Texture.hpp"
#include "../graphics/core/Shader.hpp"
#include "../graphics/core/Atlas.hpp"
#include "../graphics/core/Font.hpp"
#include "../frontend/UiDocument.hpp"
#include "../logic/scripting/scripting.h"

Assets::~Assets() {
}

Texture* Assets::getTexture(std::string name) const {
	auto found = textures.find(name);
	if (found == textures.end())
		return nullptr;
	return found->second.get();
}

void Assets::store(Texture* texture, std::string name){
	textures.emplace(name, texture);
}


Shader* Assets::getShader(std::string name) const{
	auto found = shaders.find(name);
	if (found == shaders.end())
		return nullptr;
	return found->second.get();
}

void Assets::store(Shader* shader, std::string name){
	shaders.emplace(name, shader);
}

Font* Assets::getFont(std::string name) const {
	auto found = fonts.find(name);
	if (found == fonts.end())
		return nullptr;
	return found->second.get();
}

void Assets::store(Font* font, std::string name){
	fonts.emplace(name, font);
}

Atlas* Assets::getAtlas(std::string name) const {
	auto found = atlases.find(name);
	if (found == atlases.end())
		return nullptr;
	return found->second.get();
}

void Assets::store(Atlas* atlas, std::string name){
	atlases.emplace(name, atlas);
}

audio::Sound* Assets::getSound(std::string name) const {
	auto found = sounds.find(name);
	if (found == sounds.end())
		return nullptr;
	return found->second.get();
}

void Assets::store(audio::Sound* sound, std::string name) {
	sounds.emplace(name, sound);
}

const std::vector<TextureAnimation>& Assets::getAnimations() {
	return animations;
}

void Assets::store(const TextureAnimation& animation) {
	animations.emplace_back(animation);
}

UiDocument* Assets::getLayout(std::string name) const {
	auto found = layouts.find(name);
	if (found == layouts.end())
		return nullptr;
	return found->second.get();
}

void Assets::store(UiDocument* layout, std::string name) {
	layouts[name] = std::shared_ptr<UiDocument>(layout);
}
