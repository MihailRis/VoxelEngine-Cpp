#include "AssetsLoader.h"
#include "Assets.h"

#include "asset_loaders.h"

#include <iostream>
#include <memory>

#include "../constants.h"
#include "../graphics-vk/ShaderType.h"
#include "../graphics-vk/VulkanContext.h"
#include "../graphics-vk/device/Shader.h"

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

void AssetsLoader::createDefaults(AssetsLoader& loader) {
	loader.addLoader(ASSET_SHADER, assetload::shader);
	loader.addLoader(ASSET_TEXTURE, assetload::texture);
	loader.addLoader(ASSET_FONT, assetload::font);
	loader.addLoader(ASSET_ATLAS, assetload::atlas);
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