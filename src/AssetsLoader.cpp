#include "AssetsLoader.h"
#include "Assets.h"

#include <iostream>

AssetsLoader::AssetsLoader(Assets* assets) : assets(assets) {
}

void AssetsLoader::addLoader(int tag, aloader_func func) {
	loaders[tag] = func;
}

void AssetsLoader::add(int tag, const std::string filename, const std::string alias) {
	entries.push(aloader_entry{ tag, filename, alias });
}

bool AssetsLoader::hasNext() const {
	return !entries.empty();
}

bool AssetsLoader::loadNext() {
	const aloader_entry& entry = entries.front();
	std::cout << "    loading " << entry.filename << " as " << entry.alias << std::endl;
	std::cout.flush();
	auto found = loaders.find(entry.tag);
	if (found == loaders.end()) {
		std::cerr << "unknown asset tag " << entry.tag << std::endl;
		return false;
	}
	aloader_func loader = found->second;
	bool status = loader(assets, entry.filename, entry.alias);
	entries.pop();
	return status;
}

#include "graphics/Shader.h"
#include "graphics/Texture.h"
#include "graphics/Font.h"

bool _load_shader(Assets* assets, const std::string& filename, const std::string& name) {
	Shader* shader = load_shader(filename + ".glslv", filename + ".glslf");
	if (shader == nullptr) {
		std::cerr << "failed to load shader '" << name << "'" << std::endl;
		return false;
	}
	assets->store(shader, name);
	return true;
}

bool _load_texture(Assets* assets, const std::string& filename, const std::string& name) {
	Texture* texture = load_texture(filename);
	if (texture == nullptr) {
		std::cerr << "failed to load texture '" << name << "'" << std::endl;
		return false;
	}
	assets->store(texture, name);
	return true;
}

bool _load_font(Assets* assets, const std::string& filename, const std::string& name) {
	std::vector<Texture*> pages;
	for (size_t i = 0; i <= 4; i++) {
		Texture* texture = load_texture(filename + "_" + std::to_string(i) + ".png");
		if (texture == nullptr) {
			std::cerr << "failed to load bitmap font '" << name << "' (missing page " << std::to_string(i) << ")" << std::endl;
			return false;
		}
		pages.push_back(texture);
	}
	Font* font = new Font(pages);
	assets->store(font, name);
	return true;
}

void AssetsLoader::createDefaults(AssetsLoader& loader) {
	loader.addLoader(ASSET_SHADER, _load_shader);
	loader.addLoader(ASSET_TEXTURE, _load_texture);
	loader.addLoader(ASSET_FONT, _load_font);
}
