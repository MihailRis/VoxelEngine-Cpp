#include "AssetsLoader.h"
#include "Assets.h"

#include "assetload_funcs.h"

#include <iostream>
#include <memory>

#include "../constants.h"
#include "../files/engine_paths.h"

using std::filesystem::path;
using std::unique_ptr;

AssetsLoader::AssetsLoader(Assets* assets, const ResPaths* paths) 
	: assets(assets), paths(paths) {
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
	bool status = loader(assets, paths, entry.filename, entry.alias);
	entries.pop();
	return status;
}

void AssetsLoader::createDefaults(AssetsLoader& loader) {
	loader.addLoader(ASSET_SHADER, assetload::shader);
	loader.addLoader(ASSET_TEXTURE, assetload::texture);
	loader.addLoader(ASSET_FONT, assetload::font);
	loader.addLoader(ASSET_ATLAS, assetload::atlas);
}

void AssetsLoader::addDefaults(AssetsLoader& loader, bool world) {
    loader.add(ASSET_FONT, FONTS_FOLDER"/font", "normal");
    loader.add(ASSET_SHADER, SHADERS_FOLDER"/ui", "ui");
    loader.add(ASSET_SHADER, SHADERS_FOLDER"/main", "main");
    loader.add(ASSET_SHADER, SHADERS_FOLDER"/lines", "lines");
    loader.add(ASSET_TEXTURE, TEXTURES_FOLDER"/gui/menubg.png", "gui/menubg");
    loader.add(ASSET_TEXTURE, TEXTURES_FOLDER"/gui/delete_icon.png", "gui/delete_icon");
    loader.add(ASSET_TEXTURE, TEXTURES_FOLDER"/gui/no_icon.png", "gui/no_icon");
    if (world) {
        loader.add(ASSET_SHADER, SHADERS_FOLDER"/ui3d", "ui3d");
        loader.add(ASSET_SHADER, SHADERS_FOLDER"/background", "background");
        loader.add(ASSET_SHADER, SHADERS_FOLDER"/skybox_gen", "skybox_gen");
        loader.add(ASSET_TEXTURE, TEXTURES_FOLDER"/misc/moon.png", "misc/moon");
        loader.add(ASSET_TEXTURE, TEXTURES_FOLDER"/misc/sun.png", "misc/sun");
    }
    loader.add(ASSET_ATLAS, TEXTURES_FOLDER"/blocks", "blocks");
    loader.add(ASSET_ATLAS, TEXTURES_FOLDER"/items", "items");
}

const ResPaths* AssetsLoader::getPaths() const {
	return paths;
}