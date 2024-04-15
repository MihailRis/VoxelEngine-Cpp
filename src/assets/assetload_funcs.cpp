#include "assetload_funcs.h"

#include <iostream>
#include <stdexcept>
#include <filesystem>
#include "Assets.h"
#include "AssetsLoader.h"
#include "../audio/audio.h"
#include "../files/files.h"
#include "../files/engine_paths.h"
#include "../coders/imageio.h"
#include "../coders/json.h"
#include "../coders/GLSLExtension.h"
#include "../graphics/core/Shader.h"
#include "../graphics/core/Texture.h"
#include "../graphics/core/ImageData.h"
#include "../graphics/core/Atlas.h"
#include "../graphics/core/Font.h"
#include "../graphics/core/TextureAnimation.h"
#include "../frontend/UiDocument.h"

namespace fs = std::filesystem;

static bool animation(
    Assets* assets, 
    const ResPaths* paths, 
    const std::string directory, 
    const std::string name,
    Atlas* dstAtlas
);

assetload::postfunc assetload::texture(
    AssetsLoader*,
    const ResPaths* paths,
    const std::string filename, 
    const std::string name,
    std::shared_ptr<AssetCfg>
) {
    std::shared_ptr<ImageData> image (
        imageio::read(paths->find(filename+".png").u8string()).release()
    );
    return [name, image](auto assets) {
        assets->store(Texture::from(image.get()), name);
    };
}

assetload::postfunc assetload::shader(
    AssetsLoader*,
    const ResPaths* paths,
    const std::string filename, 
    const std::string name,
    std::shared_ptr<AssetCfg>
) {
    fs::path vertexFile = paths->find(filename+".glslv");
    fs::path fragmentFile = paths->find(filename+".glslf");

    std::string vertexSource = files::read_string(vertexFile);
    std::string fragmentSource = files::read_string(fragmentFile);

    vertexSource = Shader::preprocessor->process(vertexFile, vertexSource);
    fragmentSource = Shader::preprocessor->process(fragmentFile, fragmentSource);

    return [=](auto assets) {
        assets->store(Shader::create(
            vertexFile.u8string(),
            fragmentFile.u8string(),
            vertexSource, fragmentSource
        ), name);
    };
}

static bool appendAtlas(AtlasBuilder& atlas, const fs::path& file) {
    std::string name = file.stem().string();
    // skip duplicates
    if (atlas.has(name)) {
        return false;
    }
    auto image = imageio::read(file.string());
    image->fixAlphaColor();
    atlas.add(name, image.release());

    return true;
}

assetload::postfunc assetload::atlas(
    AssetsLoader*,
    const ResPaths* paths,
    const std::string directory, 
    const std::string name,
    std::shared_ptr<AssetCfg>
) {
    AtlasBuilder builder;
    for (const auto& file : paths->listdir(directory)) {
        if (!imageio::is_read_supported(file.extension()))
            continue;
        if (!appendAtlas(builder, file))
            continue;
    }
    std::set<std::string> names = builder.getNames();
    Atlas* atlas = builder.build(2, false);
    return [=](auto assets) {
        atlas->prepare();
        assets->store(atlas, name);

        // FIXME
        for (const auto& file : names) {
            animation(assets, paths, "textures", file, atlas);
        }
    };
}

assetload::postfunc assetload::font(
    AssetsLoader*,
    const ResPaths* paths,
    const std::string filename, 
    const std::string name,
    std::shared_ptr<AssetCfg>
) {
    auto pages = std::make_shared<std::vector<std::unique_ptr<ImageData>>>();
    for (size_t i = 0; i <= 4; i++) {
        std::string name = filename + "_" + std::to_string(i) + ".png"; 
        name = paths->find(name).string();
        pages->push_back(imageio::read(name));
    }
    return [=](auto assets) {
        int res = pages->at(0)->getHeight() / 16;
        std::vector<std::unique_ptr<Texture>> textures;
        for (auto& page : *pages) {
            textures.emplace_back(Texture::from(page.get()));
        }
        assets->store(new Font(std::move(textures), res, 4), name);
    };
}

assetload::postfunc assetload::layout(
    AssetsLoader*,
    const ResPaths* paths,
    const std::string file,
    const std::string name,
    std::shared_ptr<AssetCfg> config
) {
    return [=](auto assets) {
        try {
            auto cfg = std::dynamic_pointer_cast<LayoutCfg>(config);
            auto document = UiDocument::read(cfg->env, name, file);
            assets->store(document.release(), name);
        } catch (const parsing_error& err) {
            throw std::runtime_error(
                "failed to parse layout XML '"+file+"':\n"+err.errorLog()
            );
        }
    };
}
assetload::postfunc assetload::sound(
    AssetsLoader*,
    const ResPaths* paths,
    const std::string file,
    const std::string name,
    std::shared_ptr<AssetCfg> config
) {
    auto cfg = std::dynamic_pointer_cast<SoundCfg>(config);
    bool keepPCM = cfg ? cfg->keepPCM : false;

    std::string extension = ".ogg";
    std::unique_ptr<audio::Sound> baseSound = nullptr;

    // looking for 'sound_name' as base sound
    auto soundFile = paths->find(file+extension);
    if (fs::exists(soundFile)) {
        baseSound.reset(audio::load_sound(soundFile, keepPCM));
    }
    // looking for 'sound_name_0' as base sound
    auto variantFile = paths->find(file+"_0"+extension);
    if (fs::exists(variantFile)) {
        baseSound.reset(audio::load_sound(variantFile, keepPCM));
    }

    // loading sound variants
    for (uint i = 1; ; i++) {
        auto variantFile = paths->find(file+"_"+std::to_string(i)+extension);
        if (!fs::exists(variantFile)) {
            break;
        }
        baseSound->variants.emplace_back(audio::load_sound(variantFile, keepPCM));
    }

    auto sound = baseSound.release();
    return [=](auto assets) {
        assets->store(sound, name);
    };
}

// TODO: integrate
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
            if (!appendAtlas(builder, file)) 
                continue;
        }

        std::unique_ptr<Atlas> srcAtlas (builder.build(2));
        srcAtlas->prepare();

        Texture* srcTex = srcAtlas->getTexture();
        Texture* dstTex = dstAtlas->getTexture();

        TextureAnimation animation(srcTex, dstTex);
        Frame frame;
        UVRegion region = dstAtlas->get(name);

        uint dstWidth = dstTex->getWidth();
        uint dstHeight = dstTex->getHeight();

        uint srcWidth = srcTex->getWidth();
        uint srcHeight = srcTex->getHeight();

        frame.dstPos = glm::ivec2(region.u1 * dstWidth, region.v1 * dstHeight);
        frame.size = glm::ivec2(region.u2 * dstWidth, region.v2 * dstHeight) - frame.dstPos;

        if (frameList.empty()) {
            for (const auto& elem : builder.getNames()) {
                region = srcAtlas->get(elem);
                frame.srcPos = glm::ivec2(region.u1 * srcWidth, srcHeight - region.v2 * srcHeight);
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
                frame.srcPos = glm::ivec2(region.u1 * srcWidth, srcHeight - region.v2 * srcHeight);
                animation.addFrame(frame);
            }
        }

        assets->store(srcAtlas.release(), name + "_animation");
        assets->store(animation);

        return true;
    }
    return true;
}
