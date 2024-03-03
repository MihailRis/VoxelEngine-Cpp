#include "AssetsLoader.h"
#include "Assets.h"

#include "assetload_funcs.h"

#include <iostream>
#include <memory>

#include "../constants.h"
#include "../files/engine_paths.h"
#include "../content/Content.h"
#include "../logic/scripting/scripting.h"

AssetsLoader::AssetsLoader(Assets* assets, const ResPaths* paths) 
	: assets(assets), paths(paths) {
	addLoader(ASSET_SHADER, assetload::shader);
	addLoader(ASSET_TEXTURE, assetload::texture);
	addLoader(ASSET_FONT, assetload::font);
	addLoader(ASSET_ATLAS, assetload::atlas);
    addLoader(ASSET_LAYOUT, assetload::layout);
}

void AssetsLoader::addLoader(int tag, aloader_func func) {
	loaders[tag] = func;
}

void AssetsLoader::add(int tag, const std::string filename, const std::string alias, std::shared_ptr<AssetCfg> settings) {
	entries.push(aloader_entry{ tag, filename, alias, settings});
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
	bool status = loader(*this, assets, paths, entry.filename, entry.alias, entry.config);
	entries.pop();
	return status;
}

void addLayouts(int env, const std::string& prefix, const fs::path& folder, AssetsLoader& loader) {
    if (!fs::is_directory(folder)) {
        return;
    }
    for (auto& entry : fs::directory_iterator(folder)) {
        const fs::path file = entry.path();
        if (file.extension().u8string() != ".xml")
            continue;
        std::string name = prefix+":"+file.stem().u8string();
        loader.add(ASSET_LAYOUT, file.u8string(), name, std::make_shared<LayoutCfg>(env));
    }
}

void AssetsLoader::addDefaults(AssetsLoader& loader, const Content* content) {
    loader.add(ASSET_FONT, FONTS_FOLDER"/font", "normal");
    loader.add(ASSET_SHADER, SHADERS_FOLDER"/ui", "ui");
    loader.add(ASSET_SHADER, SHADERS_FOLDER"/main", "main");
    loader.add(ASSET_SHADER, SHADERS_FOLDER"/lines", "lines");
    loader.add(ASSET_TEXTURE, TEXTURES_FOLDER"/gui/menubg.png", "gui/menubg");
    loader.add(ASSET_TEXTURE, TEXTURES_FOLDER"/gui/delete_icon.png", "gui/delete_icon");
    loader.add(ASSET_TEXTURE, TEXTURES_FOLDER"/gui/no_icon.png", "gui/no_icon");
    loader.add(ASSET_TEXTURE, TEXTURES_FOLDER"/gui/warning.png", "gui/warning");
    loader.add(ASSET_TEXTURE, TEXTURES_FOLDER"/gui/error.png", "gui/error");
    loader.add(ASSET_TEXTURE, TEXTURES_FOLDER"/gui/cross.png", "gui/cross");
    if (content) {
        loader.add(ASSET_SHADER, SHADERS_FOLDER"/ui3d", "ui3d");
        loader.add(ASSET_SHADER, SHADERS_FOLDER"/background", "background");
        loader.add(ASSET_SHADER, SHADERS_FOLDER"/skybox_gen", "skybox_gen");
        loader.add(ASSET_TEXTURE, TEXTURES_FOLDER"/misc/moon.png", "misc/moon");
        loader.add(ASSET_TEXTURE, TEXTURES_FOLDER"/misc/sun.png", "misc/sun");
        loader.add(ASSET_TEXTURE, TEXTURES_FOLDER"/gui/crosshair.png", "gui/crosshair");

        addLayouts(0, "core", loader.getPaths()->getMainRoot()/fs::path("layouts"), loader);
        for (auto& entry : content->getPacks()) {
            auto pack = entry.second.get();
            auto& info = pack->getInfo();
            fs::path folder = info.folder / fs::path("layouts");
            addLayouts(pack->getEnvironment()->getId(), info.id, folder, loader);
        }
    }
    loader.add(ASSET_ATLAS, TEXTURES_FOLDER"/blocks", "blocks");
    loader.add(ASSET_ATLAS, TEXTURES_FOLDER"/items", "items");
}

const ResPaths* AssetsLoader::getPaths() const {
	return paths;
}
