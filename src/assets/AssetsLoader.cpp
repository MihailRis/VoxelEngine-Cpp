#include "AssetsLoader.hpp"

#include <iostream>
#include <memory>
#include <utility>

#include "coders/imageio.hpp"
#include "constants.hpp"
#include "content/Content.hpp"
#include "content/ContentPack.hpp"
#include "debug/Logger.hpp"
#include "files/engine_paths.hpp"
#include "files/files.hpp"
#include "graphics/core/Texture.hpp"
#include "logic/scripting/scripting.hpp"
#include "objects/rigging.hpp"
#include "util/ThreadPool.hpp"
#include "voxels/Block.hpp"
#include "items/ItemDef.hpp"
#include "Assets.hpp"
#include "assetload_funcs.hpp"

static debug::Logger logger("assets-loader");

AssetsLoader::AssetsLoader(Assets* assets, const ResPaths* paths)
    : assets(assets), paths(paths) {
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

void AssetsLoader::add(
    AssetType tag,
    const std::string& filename,
    const std::string& alias,
    std::shared_ptr<AssetCfg> settings
) {
    if (enqueued.find({tag, alias}) != enqueued.end()){
        return;
    }
    entries.push(aloader_entry {tag, filename, alias, std::move(settings)});
    enqueued.insert({tag, alias});
}

bool AssetsLoader::hasNext() const {
    return !entries.empty();
}

aloader_func AssetsLoader::getLoader(AssetType tag) {
    auto found = loaders.find(tag);
    if (found == loaders.end()) {
        throw std::runtime_error(
            "unknown asset tag " + std::to_string(static_cast<int>(tag))
        );
    }
    return found->second;
}

void AssetsLoader::loadNext() {
    const aloader_entry& entry = entries.front();
    logger.info() << "loading " << entry.filename << " as " << entry.alias;
    try {
        aloader_func loader = getLoader(entry.tag);
        auto postfunc =
            loader(this, paths, entry.filename, entry.alias, entry.config);
        postfunc(assets);
        entries.pop();
    } catch (std::runtime_error& err) {
        logger.error() << err.what();
        auto type = entry.tag;
        std::string filename = entry.filename;
        std::string reason = err.what();
        entries.pop();
        throw assetload::error(type, std::move(filename), std::move(reason));
    }
}

void addLayouts(
    const scriptenv& env,
    const std::string& prefix,
    const fs::path& folder,
    AssetsLoader& loader
) {
    if (!fs::is_directory(folder)) {
        return;
    }
    for (auto& entry : fs::directory_iterator(folder)) {
        const fs::path& file = entry.path();
        if (file.extension().u8string() != ".xml") continue;
        std::string name = prefix + ":" + file.stem().u8string();
        loader.add(
            AssetType::LAYOUT,
            file.u8string(),
            name,
            std::make_shared<LayoutCfg>(env)
        );
    }
}

void AssetsLoader::tryAddSound(const std::string& name) {
    if (name.empty()) {
        return;
    }
    std::string file = SOUNDS_FOLDER + "/" + name;
    add(AssetType::SOUND, file, name);
}

static std::string assets_def_folder(AssetType tag) {
    switch (tag) {
        case AssetType::FONT:
            return FONTS_FOLDER;
        case AssetType::SHADER:
            return SHADERS_FOLDER;
        case AssetType::TEXTURE:
            return TEXTURES_FOLDER;
        case AssetType::ATLAS:
            return TEXTURES_FOLDER;
        case AssetType::LAYOUT:
            return LAYOUTS_FOLDER;
        case AssetType::SOUND:
            return SOUNDS_FOLDER;
        case AssetType::MODEL:
            return MODELS_FOLDER;
    }
    return "<error>";
}

void AssetsLoader::processPreload(
    AssetType tag, const std::string& name, const dv::value& map
) {
    std::string defFolder = assets_def_folder(tag);
    std::string path = defFolder + "/" + name;
    if (map == nullptr) {
        add(tag, path, name);
        return;
    }
    map.at("path").get(path);
    switch (tag) {
        case AssetType::SOUND: {
            bool keepPCM = false;
            add(tag,
                path,
                name,
                std::make_shared<SoundCfg>(map.at("keep-pcm").get(keepPCM)));
            break;
        }
        case AssetType::ATLAS: {
            std::string typeName = "atlas";
            map.at("type").get(typeName);
            auto type = AtlasType::ATLAS;
            if (typeName == "separate") {
                type = AtlasType::SEPARATE;
            }
            add(tag, path, name, std::make_shared<AtlasCfg>(type));
            break;
        }
        default:
            add(tag, path, name);
            break;
    }
}

void AssetsLoader::processPreloadList(AssetType tag, const dv::value& list) {
    if (list == nullptr) {
        return;
    }
    for (const auto& value : list) {
        switch (value.getType()) {
            case dv::value_type::string:
                processPreload(tag, value.asString(), nullptr);
                break;
            case dv::value_type::object:
                processPreload(tag, value["name"].asString(), value);
                break;
            default:
                throw std::runtime_error("invalid entry type");
        }
    }
}

void AssetsLoader::processPreloadConfig(const fs::path& file) {
    auto root = files::read_json(file);
    processPreloadList(AssetType::ATLAS, root["atlases"]);
    processPreloadList(AssetType::FONT, root["fonts"]);
    processPreloadList(AssetType::SHADER, root["shaders"]);
    processPreloadList(AssetType::TEXTURE, root["textures"]);
    processPreloadList(AssetType::SOUND, root["sounds"]);
    processPreloadList(AssetType::MODEL, root["models"]);
    // layouts are loaded automatically
}

void AssetsLoader::processPreloadConfigs(const Content* content) {
    auto preloadFile = paths->getMainRoot() / fs::path("preload.json");
    if (fs::exists(preloadFile)) {
        processPreloadConfig(preloadFile);
    }
    if (content == nullptr) {
        return;
    }
    for (auto& entry : content->getPacks()) {
        if (entry.first == "core") {
            continue;
        }
        const auto& pack = entry.second;
        auto preloadFile = pack->getInfo().folder / fs::path("preload.json");
        if (fs::exists(preloadFile)) {
            processPreloadConfig(preloadFile);
        }
    }
}

void AssetsLoader::addDefaults(AssetsLoader& loader, const Content* content) {
    loader.processPreloadConfigs(content);
    if (content) {
        for (auto& entry : content->getBlockMaterials()) {
            auto& material = *entry.second;
            loader.tryAddSound(material.stepsSound);
            loader.tryAddSound(material.placeSound);
            loader.tryAddSound(material.breakSound);
        }

        for (auto& entry : content->getPacks()) {
            auto pack = entry.second.get();
            auto& info = pack->getInfo();
            fs::path folder = info.folder / fs::path("layouts");
            addLayouts(pack->getEnvironment(), info.id, folder, loader);
        }

        for (auto& entry : content->getSkeletons()) {
            auto& skeleton = *entry.second;
            for (auto& bone : skeleton.getBones()) {
                std::string model = bone->model.name;
                size_t pos = model.rfind('.');
                if (pos != std::string::npos) {
                    model = model.substr(0, pos);
                }
                if (!model.empty()) {
                    loader.add(
                        AssetType::MODEL, MODELS_FOLDER + "/" + model, model
                    );
                }
            }
        }
        for (const auto& [_, def] : content->blocks.getDefs()) {
            if (!def->modelName.empty() &&
                def->modelName.find(':') == std::string::npos) {
                loader.add(
                    AssetType::MODEL,
                    MODELS_FOLDER + "/" + def->modelName,
                    def->modelName
                );
            }
        }
        for (const auto& [_, def] : content->items.getDefs()) {
            if (def->modelName.find(':') == std::string::npos) {
                loader.add(
                    AssetType::MODEL,
                    MODELS_FOLDER + "/" + def->modelName,
                    def->modelName
                );
            }
        }
    }
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

    assetload::postfunc operator()(const aloader_entry& entry
    ) override {
        aloader_func loadfunc = loader->getLoader(entry.tag);
        return loadfunc(
            loader,
            loader->getPaths(),
            entry.filename,
            entry.alias,
            entry.config
        );
    }
};

std::shared_ptr<Task> AssetsLoader::startTask(runnable onDone) {
    auto pool =
        std::make_shared<util::ThreadPool<aloader_entry, assetload::postfunc>>(
            "assets-loader-pool",
            [=]() { return std::make_shared<LoaderWorker>(this); },
            [=](const assetload::postfunc& func) { func(assets); }
        );
    pool->setOnComplete(std::move(onDone));
    while (!entries.empty()) {
        aloader_entry entry = std::move(entries.front());
        entries.pop();
        pool->enqueueJob(std::move(entry));
    }
    return pool;
}
