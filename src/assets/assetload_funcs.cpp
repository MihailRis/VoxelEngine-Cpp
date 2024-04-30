#include "assetload_funcs.h"
#include "Assets.h"
#include "AssetsLoader.h"
#include "../audio/audio.h"
#include "../files/files.h"
#include "../files/engine_paths.h"
#include "../coders/imageio.hpp"
#include "../coders/json.h"
#include "../coders/GLSLExtension.h"
#include "../graphics/core/Shader.hpp"
#include "../graphics/core/Texture.hpp"
#include "../graphics/core/ImageData.hpp"
#include "../graphics/core/Atlas.hpp"
#include "../graphics/core/Font.hpp"
#include "../graphics/core/TextureAnimation.hpp"
#include "../frontend/UiDocument.h"

#include <iostream>
#include <stdexcept>
#include <filesystem>

namespace fs = std::filesystem;

static bool animation(
    Assets* assets,
    const ResPaths* paths,
    const std::string& atlasName,
    const std::string& directory,
    const std::string& name,
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
        assets->store(Texture::from(image.get()).release(), name);
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

static bool append_atlas(AtlasBuilder& atlas, const fs::path& file) {
    std::string name = file.stem().string();
    // skip duplicates
    if (atlas.has(name)) {
        return false;
    }
    auto image = imageio::read(file.string());
    image->fixAlphaColor();
    atlas.add(name, std::move(image));
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
        if (!append_atlas(builder, file))
            continue;
    }
    std::set<std::string> names = builder.getNames();
    Atlas* atlas = builder.build(2, false).release();
    return [=](auto assets) {
        atlas->prepare();
        assets->store(atlas, name);
        for (const auto& file : names) {
            animation(assets, paths, name, directory, file, atlas);
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

    if (baseSound == nullptr) {
        throw std::runtime_error("could not to find sound: " + file);
    }
    auto sound = baseSound.release();
    return [=](auto assets) {
        assets->store(sound, name);
    };
}

static void read_anim_file(
    const std::string& animFile,
    std::vector<std::pair<std::string, int>>& frameList
) {
    auto root = files::read_json(animFile);
    auto frameArr = root->list("frames");
    float frameDuration = DEFAULT_FRAME_DURATION;
    std::string frameName;

    if (frameArr) {
        for (size_t i = 0; i < frameArr->size(); i++) {
            auto currentFrame = frameArr->list(i);

            frameName = currentFrame->str(0);
            if (currentFrame->size() > 1) {
                frameDuration = currentFrame->integer(1);
            }
            frameList.emplace_back(frameName, frameDuration);
        }
    }
}

static TextureAnimation create_animation(
    Atlas* srcAtlas,
    Atlas* dstAtlas,
    const std::string& name,
    const std::set<std::string>& frameNames,
    const std::vector<std::pair<std::string, int>>& frameList
) {
    Texture* srcTex = srcAtlas->getTexture();
    Texture* dstTex = dstAtlas->getTexture();
    UVRegion region = dstAtlas->get(name);

    TextureAnimation animation(srcTex, dstTex);
    Frame frame;

    uint dstWidth = dstTex->getWidth();
    uint dstHeight = dstTex->getHeight();

    uint srcWidth = srcTex->getWidth();
    uint srcHeight = srcTex->getHeight();

    frame.dstPos = glm::ivec2(region.u1 * dstWidth, region.v1 * dstHeight);
    frame.size = glm::ivec2(region.u2 * dstWidth, region.v2 * dstHeight) - frame.dstPos;

    for (const auto& elem : frameList) {
        if (!srcAtlas->has(elem.first)) {
            std::cerr << "Unknown frame name: " << elem.first << std::endl;
            continue;
        }
        region = srcAtlas->get(elem.first);
        if (elem.second > 0) {
            frame.duration = static_cast<float>(elem.second) / 1000.0f;
        }
        frame.srcPos = glm::ivec2(region.u1 * srcWidth, srcHeight - region.v2 * srcHeight);
        animation.addFrame(frame);
    }
    return animation;
}

inline bool contains(
    const std::vector<std::pair<std::string, int>>& frameList,
    const std::string& frameName
) {
    for (const auto& elem : frameList) {
        if (frameName == elem.first) {
            return true;
        }
    }
    return false;
}

static bool animation(
    Assets* assets, 
    const ResPaths* paths,
    const std::string& atlasName, 
    const std::string& directory, 
    const std::string& name,
    Atlas* dstAtlas
) {
    std::string animsDir = directory + "/animation";

    for (const auto& folder : paths->listdir(animsDir)) {
        if (!fs::is_directory(folder)) continue;
        if (folder.filename().u8string() != name) continue;
        if (fs::is_empty(folder)) continue;
        
        AtlasBuilder builder;
        append_atlas(builder, paths->find(directory + "/" + name + ".png"));

        std::vector<std::pair<std::string, int>> frameList;
        std::string animFile = folder.u8string() + "/animation.json";
        if (fs::exists(animFile)) {
            read_anim_file(animFile, frameList);
        }
        for (const auto& file : paths->listdir(animsDir + "/" + name)) {
            if (!frameList.empty() && !contains(frameList, file.stem())) {
                continue;
            }
            if (!append_atlas(builder, file)) 
                continue;
        }
        auto srcAtlas = builder.build(2, true);
        if (frameList.empty()) {
            for (const auto& frameName : builder.getNames()) {
                frameList.emplace_back(frameName, 0);
            }
        }
        auto animation = create_animation(
            srcAtlas.get(), dstAtlas, name, builder.getNames(), frameList
        );
        assets->store(srcAtlas.release(), atlasName + "/" + name + "_animation");
        assets->store(animation);
        return true;
    }
    return true;
}
