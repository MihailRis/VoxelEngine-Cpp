#include "Assets.h"

#include "../audio/audio.h"
#include "../graphics/Texture.h"
#include "../graphics/Shader.h"
#include "../graphics/Atlas.h"
#include "../graphics/Font.h"
#include "../frontend/UiDocument.h"
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
	layouts.emplace(name, layout);
}

void Assets::extend(const Assets& assets) {
    for (auto entry : assets.textures) {
        textures[entry.first] = entry.second;
    }
    for (auto entry : assets.shaders) {
        shaders[entry.first] = entry.second;
    }
    for (auto entry : assets.fonts) {
        fonts[entry.first] = entry.second;
    }
    for (auto entry : assets.atlases) {
        atlases[entry.first] = entry.second;
    }
	for (auto entry : assets.layouts) {
		layouts[entry.first] = entry.second;
	}
	for (auto entry : assets.sounds) {
		sounds[entry.first] = entry.second;
	}
    animations.clear();
	for (auto entry : assets.animations) {
		animations.emplace_back(entry);
	}
}
