#include "AssetsLoader.hpp"

#include "Assets.hpp"
#include "assetload_funcs.hpp"
#include "../util/ThreadPool.hpp"
#include "../constants.hpp"
#include "../data/dynamic.hpp"
#include "../debug/Logger.hpp"
#include "../coders/imageio.hpp"
#include "../files/files.hpp"
#include "../files/engine_paths.hpp"
#include "../content/Content.hpp"
#include "../content/ContentPack.hpp"
#include "../voxels/Block.hpp"
#include "../objects/rigging.hpp"
#include "../graphics/core/Texture.hpp"
#include "../logic/scripting/scripting.hpp"

#include <iostream>
#include <memory>
#include <utility>

static debug::Logger logger("assets-loader");

AssetsLoader::AssetsLoader(Assets* assets, const ResPaths* paths) 
  : assets(assets), paths(paths) 
{
    addLoader(AssetType::SHADER, assetload::shader);
    addLoader(AssetType::TEXTURE, assetload::texture);
    addLoader(AssetType::FONT, assetload::font);
    addLoader(AssetType::ATLAS, assetload::atlas);
    addLoader(AssetType::LAYOUT, assetload::layout);
    addLoader(AssetType::SOUND, assetload::sound);
    addLoader(AssetType::MODEL, assetload::model);
}

void AssetsLoader::addLoader(AssetType tag, aloader_func func) {
    loaders[tag] = std::move(func);
}

void AssetsLoader::add(AssetType tag, const std::string& filename, const std::string& alias, std::shared_ptr<AssetCfg> settings) {
    entries.push(aloader_entry{tag, filename, alias, std::move(settings)});
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

void addLayouts(const scriptenv& env, const std::string& prefix, const fs::path& folder, AssetsLoader& loader) {
    if (!fs::is_directory(folder)) {
        return;
    }
    for (auto& entry : fs::directory_iterator(folder)) {
        const fs::path& file = entry.path();
        if (file.extension().u8string() != ".xml")
            continue;
        std::string name = prefix+":"+file.stem().u8string();
        loader.add(AssetType::LAYOUT, file.u8string(), name, std::make_shared<LayoutCfg>(env));
    }
}

void AssetsLoader::tryAddSound(const std::string& name) {
    if (name.empty()) {
        return;
    }
    std::string file = SOUNDS_FOLDER+"/"+name;
    add(AssetType::SOUND, file, name);
}

static std::string assets_def_folder(AssetType tag) {
    switch (tag) {
        case AssetType::FONT: return FONTS_FOLDER;
        case AssetType::SHADER: return SHADERS_FOLDER;
        case AssetType::TEXTURE: return TEXTURES_FOLDER;
        case AssetType::ATLAS: return TEXTURES_FOLDER;
        case AssetType::LAYOUT: return LAYOUTS_FOLDER;
        case AssetType::SOUND: return SOUNDS_FOLDER;
        case AssetType::MODEL: return MODELS_FOLDER;
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
        case AssetType::SOUND:
            add(tag, path, name, std::make_shared<SoundCfg>(
                map->get("keep-pcm", false)
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
        switch (static_cast<dynamic::Type>(value.index())) {
            case dynamic::Type::string:
                processPreload(tag, std::get<std::string>(value), nullptr);
                break;
            case dynamic::Type::map: {
                auto map = std::get<dynamic::Map_sptr>(value);
                auto name = map->get<std::string>("name");
                processPreload(tag, name, map.get());
                break;
            }
            default:
                throw std::runtime_error("invalid entry type");
        }
    }
}

void AssetsLoader::processPreloadConfig(const fs::path& file) {
    auto root = files::read_json(file);
    processPreloadList(AssetType::FONT, root->list("fonts").get());
    processPreloadList(AssetType::SHADER, root->list("shaders").get());
    processPreloadList(AssetType::TEXTURE, root->list("textures").get());
    processPreloadList(AssetType::SOUND, root->list("sounds").get());
    processPreloadList(AssetType::MODEL, root->list("models").get());
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
    loader.add(AssetType::FONT, FONTS_FOLDER+"/font", "normal");
    loader.add(AssetType::SHADER, SHADERS_FOLDER+"/ui", "ui");
    loader.add(AssetType::SHADER, SHADERS_FOLDER+"/main", "main");
    loader.add(AssetType::SHADER, SHADERS_FOLDER+"/entity", "entity");
    loader.add(AssetType::SHADER, SHADERS_FOLDER+"/lines", "lines");
    loader.add(AssetType::TEXTURE, TEXTURES_FOLDER+"/gui/menubg", "gui/menubg");
    loader.add(AssetType::TEXTURE, TEXTURES_FOLDER+"/gui/delete_icon", "gui/delete_icon");
    loader.add(AssetType::TEXTURE, TEXTURES_FOLDER+"/gui/no_icon", "gui/no_icon");
    loader.add(AssetType::TEXTURE, TEXTURES_FOLDER+"/gui/no_world_icon", "gui/no_world_icon");
    loader.add(AssetType::TEXTURE, TEXTURES_FOLDER+"/gui/warning", "gui/warning");
    loader.add(AssetType::TEXTURE, TEXTURES_FOLDER+"/gui/error", "gui/error");
    loader.add(AssetType::TEXTURE, TEXTURES_FOLDER+"/gui/cross", "gui/cross");
    loader.add(AssetType::TEXTURE, TEXTURES_FOLDER+"/gui/refresh", "gui/refresh");
    if (content) {
        loader.processPreloadConfigs(content);

        for (auto& entry : content->getBlockMaterials()) {
            auto& material = *entry.second;
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

        for (auto& entry : content->getSkeletons()) {
            auto& skeleton = *entry.second;
            for (auto& bone : skeleton.getBones()) {
                auto& model = bone->model.name;
                if (!model.empty()) {
                    loader.add(AssetType::MODEL, MODELS_FOLDER+"/"+model, model);
                }
            }
        }
    }
    loader.add(AssetType::ATLAS, TEXTURES_FOLDER+"/blocks", "blocks");
    loader.add(AssetType::ATLAS, TEXTURES_FOLDER+"/items", "items");
}

bool AssetsLoader::loadExternalTexture(
    Assets* assets,
    const std::string& name,
    const std::vector<std::filesystem::path>& alternatives
) {
    if (assets->get<Texture>(name) != nullptr) {
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
    pool->setOnComplete(std::move(onDone));
    while (!entries.empty()) {
        const aloader_entry& entry = entries.front();
        auto ptr = std::make_shared<aloader_entry>(entry);
        pool->enqueueJob(ptr);
        entries.pop();
    }
    return pool;
}
