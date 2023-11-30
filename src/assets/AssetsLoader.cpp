#include "AssetsLoader.h"
#include "Assets.h"

#include <iostream>
#include <memory>

#include "../constants.h"

using std::filesystem::path;
using std::unique_ptr;

AssetsLoader::AssetsLoader(Assets* assets, path resdir) 
			 : assets(assets), resdir(resdir) {
}

void AssetsLoader::addLoader(int tag, aloader_func func) {
	loaders[tag] = func;
}

void AssetsLoader::add(int tag, const path filename, const std::string alias) {
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

#include "../coders/png.h"
#include "../graphics/Shader.h"
#include "../graphics/ImageData.h"
#include "../graphics/Texture.h"
#include "../graphics/Atlas.h"
#include "../graphics/Font.h"

bool _load_shader(Assets* assets, const path& filename, const std::string& name) {
	Shader* shader = Shader::loadShader(filename.string() + ".glslv", filename.string() + ".glslf");
	if (shader == nullptr) {
		std::cerr << "failed to load shader '" << name << "'" << std::endl;
		return false;
	}
	assets->store(shader, name);
	return true;
}

bool _load_texture(Assets* assets, const path& filename, const std::string& name) {
	Texture* texture = png::load_texture(filename.string());
	if (texture == nullptr) {
		std::cerr << "failed to load texture '" << name << "'" << std::endl;
		return false;
	}
	assets->store(texture, name);
	return true;
}

bool _load_atlas(Assets* assets, const path& filename, const std::string& name) {
	AtlasBuilder builder;
	for (const auto& entry : std::filesystem::directory_iterator(filename)) {
		std::filesystem::path file = entry.path();
		if (file.extension() == ".png") {
			std::string name = file.stem().string();
			std::unique_ptr<ImageData> image (png::load_image(file.string()));
			image->fixAlphaColor();
			builder.add(name, image.release());
		}
	}
	Atlas* atlas = builder.build(2);
	assets->store(atlas, name);
	return true;
}

bool _load_font(Assets* assets, const path& filename, const std::string& name) {
	std::vector<Texture*> pages;
	for (size_t i = 0; i <= 4; i++) {
		Texture* texture = png::load_texture(filename.string() + "_" + std::to_string(i) + ".png");
		if (texture == nullptr) {
			std::cerr << "failed to load bitmap font '" << name << "' (missing page " << std::to_string(i) << ")" << std::endl;
			return false;
		}
		pages.push_back(texture);
	}
	Font* font = new Font(pages, pages[0]->height / 16);
	assets->store(font, name);
	return true;
}

void AssetsLoader::createDefaults(AssetsLoader& loader) {
	loader.addLoader(ASSET_SHADER, _load_shader);
	loader.addLoader(ASSET_TEXTURE, _load_texture);
	loader.addLoader(ASSET_FONT, _load_font);
	loader.addLoader(ASSET_ATLAS, _load_atlas);
}

void AssetsLoader::addDefaults(AssetsLoader& loader) {
	path resdir = loader.getDirectory();
	loader.add(ASSET_SHADER, resdir/path(SHADERS_FOLDER"/main"), "main");
	loader.add(ASSET_SHADER, resdir/path(SHADERS_FOLDER"/lines"), "lines");
	loader.add(ASSET_SHADER, resdir/path(SHADERS_FOLDER"/ui"), "ui");
	loader.add(ASSET_SHADER, resdir/path(SHADERS_FOLDER"/ui3d"), "ui3d");
	loader.add(ASSET_SHADER, resdir/path(SHADERS_FOLDER"/background"), "background");
	loader.add(ASSET_SHADER, resdir/path(SHADERS_FOLDER"/skybox_gen"), "skybox_gen");

	loader.add(ASSET_ATLAS, resdir/path(TEXTURES_FOLDER"/blocks"), "blocks");
	loader.add(ASSET_TEXTURE, resdir/path(TEXTURES_FOLDER"/menubg.png"), "menubg");

	loader.add(ASSET_FONT, resdir/path(FONTS_FOLDER"/font"), "normal");
}

path AssetsLoader::getDirectory() const {
	return resdir;
}