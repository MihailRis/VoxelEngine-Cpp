#include "assetload_funcs.h"

#include <iostream>
#include <filesystem>
#include "Assets.h"
#include "AssetsLoader.h"
#include "../audio/audio.h"
#include "../files/files.h"
#include "../files/engine_paths.h"
#include "../coders/png.h"
#include "../coders/json.h"
#include "../graphics/Shader.h"
#include "../graphics/Texture.h"
#include "../graphics/ImageData.h"
#include "../graphics/Atlas.h"
#include "../graphics/Font.h"
#include "../graphics/TextureAnimation.h"
#include "../frontend/UiDocument.h"
#include "../logic/scripting/scripting.h"

namespace fs = std::filesystem;

static bool animation(
    Assets* assets, 
    const ResPaths* paths, 
    const std::string directory, 
    const std::string name,
    Atlas* dstAtlas
);

bool assetload::texture(
    AssetsLoader&,
    Assets* assets,
    const ResPaths* paths,
    const std::string filename, 
    const std::string name,
    std::shared_ptr<AssetCfg>
) {
    std::unique_ptr<Texture> texture(
        png::load_texture(paths->find(filename).u8string())
    );
    if (texture == nullptr) {
        std::cerr << "failed to load texture '" << name << "'" << std::endl;
        return false;
    }
    assets->store(texture.release(), name);
    return true;
}

bool assetload::shader(
    AssetsLoader&,
    Assets* assets,
    const ResPaths* paths,
    const std::string filename, 
    const std::string name,
    std::shared_ptr<AssetCfg>
) {
    fs::path vertexFile = paths->find(filename+".glslv");
    fs::path fragmentFile = paths->find(filename+".glslf");

    std::string vertexSource = files::read_string(vertexFile);
    std::string fragmentSource = files::read_string(fragmentFile);

    Shader* shader = Shader::create(
        vertexFile.string(),
        fragmentFile.string(),
        vertexSource, fragmentSource
    );

    if (shader == nullptr) {
        std::cerr << "failed to load shader '" << name << "'" << std::endl;
        return false;
    }
    assets->store(shader, name);
    return true;
}

static bool appendAtlas(AtlasBuilder& atlas, const fs::path& file) {
    // png is only supported format
    if (file.extension() != ".png")
        return false;
    std::string name = file.stem().string();
    // skip duplicates
    if (atlas.has(name)) {
        return false;
    }
    std::unique_ptr<ImageData> image(png::load_image(file.string()));
    if (image == nullptr) {
        std::cerr << "could not to load " << file.string() << std::endl;
        return false;
    }
    image->fixAlphaColor();
    atlas.add(name, image.release());

    return true;
}

bool assetload::atlas(
    AssetsLoader&,
    Assets* assets, 
    const ResPaths* paths,
    const std::string directory, 
    const std::string name,
    std::shared_ptr<AssetCfg>
) {
    AtlasBuilder builder;
    for (const auto& file : paths->listdir(directory)) {
        if (!appendAtlas(builder, file)) continue;
    }
    Atlas* atlas = builder.build(2);
    assets->store(atlas, name);
    for (const auto& file : builder.getNames()) {
        animation(assets, paths, "textures", file, atlas);
    }
    return true;
}

bool assetload::font(
    AssetsLoader&,
    Assets* assets, 
    const ResPaths* paths,
    const std::string filename, 
    const std::string name,
    std::shared_ptr<AssetCfg>
) {
    std::vector<std::unique_ptr<Texture>> pages;
    for (size_t i = 0; i <= 4; i++) {
        std::string name = filename + "_" + std::to_string(i) + ".png"; 
        name = paths->find(name).string();
        std::unique_ptr<Texture> texture (png::load_texture(name));
        if (texture == nullptr) {
            std::cerr << "failed to load bitmap font '" << name;
            std::cerr << "' (missing page " << std::to_string(i) << ")";
            std::cerr << std::endl;
            return false;
        }
        pages.push_back(std::move(texture));
    }
    int res = pages[0]->height / 16;
    assets->store(new Font(std::move(pages), res, 4), name);
    return true;
}

bool assetload::layout(
    AssetsLoader& loader,
    Assets* assets,
    const ResPaths* paths,
    const std::string file,
    const std::string name,
    std::shared_ptr<AssetCfg> config
) {
    try {
        auto cfg = dynamic_cast<LayoutCfg*>(config.get());
        auto document = UiDocument::read(loader, cfg->env, name, file);
        assets->store(document.release(), name);
        return true;
    } catch (const parsing_error& err) {
        std::cerr << "failed to parse layout XML '" << file << "'" << std::endl;
        std::cerr << err.errorLog() << std::endl;
        return false;
    }
}
bool assetload::sound(
    AssetsLoader& loader,
    Assets* assets,
    const ResPaths* paths,
    const std::string file,
    const std::string name,
    std::shared_ptr<AssetCfg> config
) {
    auto cfg = dynamic_cast<SoundCfg*>(config.get());
    auto soundFile = paths->find(file);

    bool keepPCM = cfg ? cfg->keepPCM : false;

    size_t lastindex = file.find_last_of("."); 
    std::string extension = file.substr(lastindex);
    std::string extensionless = file.substr(0, lastindex);
    try {
        std::unique_ptr<audio::Sound> baseSound = nullptr;
        if (fs::exists(soundFile)) {
            baseSound.reset(audio::load_sound(soundFile, keepPCM));
        }
        auto variantFile = paths->find(extensionless+"_0"+extension);
        if (fs::exists(variantFile)) {
            baseSound.reset(audio::load_sound(variantFile, keepPCM));
        }
        for (uint i = 1; ; i++) {
            auto variantFile = paths->find(extensionless+"_"+std::to_string(i)+extension);
            if (!fs::exists(variantFile)) {
                break;
            }
            baseSound->variants.emplace_back(audio::load_sound(variantFile, keepPCM));
        }
        assets->store(baseSound.release(), name);
    } 
    catch (std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
    }
    return true;
}

static bool animation(
    Assets* assets, 
    const ResPaths* paths, 
    const std::string directory, 
    const std::string name,
    Atlas* dstAtlas
) {
    std::string animsDir = directory + "/animations";
    std::string blocksDir = directory + "/blocks";

    for (const auto& folder : paths->listdir(animsDir)) {
        if (!fs::is_directory(folder)) continue;
        if (folder.filename().string() != name) continue;
        if (fs::is_empty(folder)) continue;
        
        AtlasBuilder builder;
        appendAtlas(builder, paths->find(blocksDir + "/" + name + ".png"));

        std::string animFile = folder.string() + "/animation.json";

        std::vector<std::pair<std::string, float>> frameList;

        if (fs::exists(animFile)) {
            auto root = files::read_json(animFile);

            auto frameArr = root->list("frames");

            float frameDuration = DEFAULT_FRAME_DURATION;
            std::string frameName;

            if (frameArr) {
                for (size_t i = 0; i < frameArr->size(); i++) {
                    auto currentFrame = frameArr->list(i);

                    frameName = currentFrame->str(0);
                    if (currentFrame->size() > 1) 
                        frameDuration = static_cast<float>(currentFrame->integer(1)) / 1000;

                    frameList.emplace_back(frameName, frameDuration);
                }
            }
        }
        for (const auto& file : paths->listdir(animsDir + "/" + name)) {
            if (!frameList.empty()) {
                bool contains = false;
                for (const auto& elem : frameList) {
                    if (file.stem() == elem.first) {
                        contains = true;
                        break;
                    }
                }
                if (!contains) continue;
            }
            if (!appendAtlas(builder, file)) continue;
        }

        std::unique_ptr<Atlas> srcAtlas (builder.build(2));

        Texture* srcTex = srcAtlas->getTexture();
        Texture* dstTex = dstAtlas->getTexture();

        TextureAnimation animation(srcTex, dstTex);
        Frame frame;
        UVRegion region = dstAtlas->get(name);

        frame.dstPos = glm::ivec2(region.u1 * dstTex->width, region.v1 * dstTex->height);
        frame.size = glm::ivec2(region.u2 * dstTex->width, region.v2 * dstTex->height) - frame.dstPos;

        if (frameList.empty()) {
            for (const auto& elem : builder.getNames()) {
                region = srcAtlas->get(elem);
                frame.srcPos = glm::ivec2(region.u1 * srcTex->width, srcTex->height - region.v2 * srcTex->height);
                animation.addFrame(frame);
            }
        }
        else {
            for (const auto& elem : frameList) {
                if (!srcAtlas->has(elem.first)) {
                    std::cerr << "Unknown frame name: " << elem.first << std::endl;
                    continue;
                }
                region = srcAtlas->get(elem.first);
                frame.duration = elem.second;
                frame.srcPos = glm::ivec2(region.u1 * srcTex->width, srcTex->height - region.v2 * srcTex->height);
                animation.addFrame(frame);
            }
        }

        assets->store(srcAtlas.release(), name + "_animation");
        assets->store(animation);

        return true;
    }
    return true;
}
