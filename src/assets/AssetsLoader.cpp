#include "AssetsLoader.h"
#include "Assets.h"

#include "assetload_funcs.h"

#include <iostream>
#include <memory>

#include "../util/ThreadPool.hpp"

#include "../constants.h"
#include "../data/dynamic.h"
#include "../debug/Logger.hpp"
#include "../coders/imageio.hpp"
#include "../files/files.h"
#include "../files/engine_paths.h"
#include "../content/Content.h"
#include "../content/ContentPack.h"
#include "../graphics/core/Texture.hpp"
#include "../logic/scripting/scripting.h"

static debug::Logger logger("assets-loader");

AssetsLoader::AssetsLoader(Assets* assets, const ResPaths* paths) 
  : assets(assets), paths(paths) 
{
    addLoader(AssetType::shader, assetload::shader);
    addLoader(AssetType::texture, assetload::texture);
    addLoader(AssetType::font, assetload::font);
    addLoader(AssetType::atlas, assetload::atlas);
    addLoader(AssetType::layout, assetload::layout);
    addLoader(AssetType::sound, assetload::sound);
}

void AssetsLoader::addLoader(AssetType tag, aloader_func func) {
    loaders[tag] = func;
}

void AssetsLoader::add(AssetType tag, const std::string filename, const std::string alias, std::shared_ptr<AssetCfg> settings) {
    entries.push(aloader_entry{tag, filename, alias, settings});
}

bool AssetsLoader::hasNext() const {
    return !entries.empty();
}

aloader_func AssetsLoader::getLoader(AssetType tag) {
    auto found = loaders.find(tag);
    if (found == loaders.end()) {
        throw std::runtime_error(
            "unknown asset tag "+std::to_string(static_cast<int>(tag))
        );
    }
    return found->second;
}

bool AssetsLoader::loadNext() {
    const aloader_entry& entry = entries.front();
    logger.info() << "loading " << entry.filename << " as " << entry.alias;
    try {
        aloader_func loader = getLoader(entry.tag);
        auto postfunc = loader(this, paths, entry.filename, entry.alias, entry.config);
        postfunc(assets);
        entries.pop();
        return true;
    } catch (std::runtime_error& err) {
        logger.error() << err.what();
        entries.pop();
        return false;
    }
}

void addLayouts(scriptenv env, const std::string& prefix, const fs::path& folder, AssetsLoader& loader) {
    if (!fs::is_directory(folder)) {
        return;
    }
    for (auto& entry : fs::directory_iterator(folder)) {
        const fs::path file = entry.path();
        if (file.extension().u8string() != ".xml")
            continue;
        std::string name = prefix+":"+file.stem().u8string();
        loader.add(AssetType::layout, file.u8string(), name, std::make_shared<LayoutCfg>(env));
    }
}

void AssetsLoader::tryAddSound(std::string name) {
    if (name.empty()) {
        return;
    }
    std::string file = SOUNDS_FOLDER+"/"+name;
    add(AssetType::sound, file, name);
}

static std::string assets_def_folder(AssetType tag) {
    switch (tag) {
        case AssetType::font: return FONTS_FOLDER;
        case AssetType::shader: return SHADERS_FOLDER;
        case AssetType::texture: return TEXTURES_FOLDER;
        case AssetType::atlas: return TEXTURES_FOLDER;
        case AssetType::layout: return LAYOUTS_FOLDER;
        case AssetType::sound: return SOUNDS_FOLDER;
    }
    return "<error>";
}

void AssetsLoader::processPreload(
    AssetType tag, 
    const std::string& name, 
    dynamic::Map* map
) {
    std::string defFolder = assets_def_folder(tag);
    std::string path = defFolder+"/"+name;
    if (map == nullptr) {
        add(tag, path, name);
        return;
    }
    map->str("path", path);
    switch (tag) {
        case AssetType::sound:
            add(tag, path, name, std::make_shared<SoundCfg>(
                map->getBool("keep-pcm", false)
            ));
            break;
        default:
            add(tag, path, name);
            break;
    }
}

void AssetsLoader::processPreloadList(AssetType tag, dynamic::List* list) {
    if (list == nullptr) {
        return;
    }
    for (uint i = 0; i < list->size(); i++) {
        auto value = list->get(i);
        switch (value->type) {
            case dynamic::valtype::string:
                processPreload(tag, std::get<std::string>(value->value), nullptr);
                break;
            case dynamic::valtype::map: {
                auto map = std::get<dynamic::Map*>(value->value);
                auto name = map->getStr("name");
                processPreload(tag, name, map);
                break;
            }
            default:
                throw std::runtime_error("invalid entry type");
        }
    }
}

void AssetsLoader::processPreloadConfig(fs::path file) {
    auto root = files::read_json(file);
    processPreloadList(AssetType::font, root->list("fonts"));
    processPreloadList(AssetType::shader, root->list("shaders"));
    processPreloadList(AssetType::texture, root->list("textures"));
    processPreloadList(AssetType::sound, root->list("sounds"));
    // layouts are loaded automatically
}

void AssetsLoader::processPreloadConfigs(const Content* content) {
    for (auto& entry : content->getPacks()) {
        const auto& pack = entry.second;
        auto preloadFile = pack->getInfo().folder / fs::path("preload.json");
        if (fs::exists(preloadFile)) {
            processPreloadConfig(preloadFile);
        }
    }
    auto preloadFile = paths->getMainRoot()/fs::path("preload.json");
    if (fs::exists(preloadFile)) {
        processPreloadConfig(preloadFile);
    }
}

void AssetsLoader::addDefaults(AssetsLoader& loader, const Content* content) {
    loader.add(AssetType::font, FONTS_FOLDER+"/font", "normal");
    loader.add(AssetType::shader, SHADERS_FOLDER+"/ui", "ui");
    loader.add(AssetType::shader, SHADERS_FOLDER+"/main", "main");
    loader.add(AssetType::shader, SHADERS_FOLDER+"/lines", "lines");
    loader.add(AssetType::texture, TEXTURES_FOLDER+"/gui/menubg", "gui/menubg");
    loader.add(AssetType::texture, TEXTURES_FOLDER+"/gui/delete_icon", "gui/delete_icon");
    loader.add(AssetType::texture, TEXTURES_FOLDER+"/gui/no_icon", "gui/no_icon");
    loader.add(AssetType::texture, TEXTURES_FOLDER+"/gui/no_world_icon", "gui/no_world_icon");
    loader.add(AssetType::texture, TEXTURES_FOLDER+"/gui/warning", "gui/warning");
    loader.add(AssetType::texture, TEXTURES_FOLDER+"/gui/error", "gui/error");
    loader.add(AssetType::texture, TEXTURES_FOLDER+"/gui/cross", "gui/cross");
    if (content) {
        loader.processPreloadConfigs(content);

        for (auto& entry : content->getBlockMaterials()) {
            auto& material = entry.second;
            loader.tryAddSound(material.stepsSound);
            loader.tryAddSound(material.placeSound);
            loader.tryAddSound(material.breakSound);
        }

        addLayouts(0, "core", loader.getPaths()->getMainRoot()/fs::path("layouts"), loader);
        for (auto& entry : content->getPacks()) {
            auto pack = entry.second.get();
            auto& info = pack->getInfo();
            fs::path folder = info.folder / fs::path("layouts");
            addLayouts(pack->getEnvironment(), info.id, folder, loader);
        }
    }
    loader.add(AssetType::atlas, TEXTURES_FOLDER+"/blocks", "blocks");
    loader.add(AssetType::atlas, TEXTURES_FOLDER+"/items", "items");
}

bool AssetsLoader::loadExternalTexture(
    Assets* assets,
    const std::string& name,
    std::vector<std::filesystem::path> alternatives
) {
    if (assets->getTexture(name) != nullptr) {
        return true;
    }
    for (auto& path : alternatives) {
        if (fs::exists(path)) {
            try {
                auto image = imageio::read(path.string());
                assets->store(Texture::from(image.get()), name);
                return true;
            } catch (const std::exception& err) {
                logger.error() << "error while loading external " 
                    << path.u8string() << ": " << err.what();
            }
        }
    }
    return false;
}

const ResPaths* AssetsLoader::getPaths() const {
    return paths;
}

class LoaderWorker : public util::Worker<aloader_entry, assetload::postfunc> {
    AssetsLoader* loader;
public:
    LoaderWorker(AssetsLoader* loader) : loader(loader) {
    }

    assetload::postfunc operator()(const std::shared_ptr<aloader_entry>& entry) override {
        aloader_func loadfunc = loader->getLoader(entry->tag);
        return loadfunc(loader, loader->getPaths(), entry->filename, entry->alias, entry->config);
    }
};

std::shared_ptr<Task> AssetsLoader::startTask(runnable onDone) {
    auto pool = std::make_shared<
        util::ThreadPool<aloader_entry, assetload::postfunc>
    >(
        "assets-loader-pool", 
        [=](){return std::make_shared<LoaderWorker>(this);},
        [=](assetload::postfunc& func) {
            func(assets);
        }
    );
    pool->setOnComplete(onDone);
    while (!entries.empty()) {
        const aloader_entry& entry = entries.front();
        auto ptr = std::make_shared<aloader_entry>(entry);
        pool->enqueueJob(ptr);
        entries.pop();
    }
    return pool;
}
