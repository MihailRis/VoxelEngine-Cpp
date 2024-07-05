#include "assetload_funcs.hpp"

#include "Assets.hpp"
#include "AssetsLoader.hpp"
#include "../data/dynamic.hpp"
#include "../audio/audio.hpp"
#include "../files/files.hpp"
#include "../files/engine_paths.hpp"
#include "../coders/commons.hpp"
#include "../coders/imageio.hpp"
#include "../coders/json.hpp"
#include "../coders/obj.hpp"
#include "../coders/GLSLExtension.hpp"
#include "../graphics/core/Shader.hpp"
#include "../graphics/core/Texture.hpp"
#include "../graphics/core/ImageData.hpp"
#include "../graphics/core/Atlas.hpp"
#include "../graphics/core/Font.hpp"
#include "../graphics/core/Model.hpp"
#include "../graphics/core/TextureAnimation.hpp"
#include "../objects/rigging.hpp"
#include "../frontend/UiDocument.hpp"
#include "../constants.hpp"

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
    const std::string& filename,
    const std::string& name,
    const std::shared_ptr<AssetCfg>&
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
    const std::string& filename,
    const std::string& name,
    const std::shared_ptr<AssetCfg>&
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
    const std::string& directory,
    const std::string& name,
    const std::shared_ptr<AssetCfg>&
) {
    AtlasBuilder builder;
    for (const auto& file : paths->listdir(directory)) {
        if (!imageio::is_read_supported(file.extension().u8string()))
            continue;
        if (!append_atlas(builder, file))
            continue;
    }
    std::set<std::string> names = builder.getNames();
    Atlas* atlas = builder.build(2, false).release();
    return [=](auto assets) {
        atlas->prepare();
        assets->store(std::unique_ptr<Atlas>(atlas), name);
        for (const auto& file : names) {
            animation(assets, paths, name, directory, file, atlas);
        }
    };
}

assetload::postfunc assetload::font(
    AssetsLoader*,
    const ResPaths* paths,
    const std::string& filename,
    const std::string& name,
    const std::shared_ptr<AssetCfg>&
) {
    auto pages = std::make_shared<std::vector<std::unique_ptr<ImageData>>>();
    for (size_t i = 0; i <= 4; i++) {
        std::string pagefile = filename + "_" + std::to_string(i) + ".png"; 
        pagefile = paths->find(pagefile).string();
        pages->push_back(imageio::read(pagefile));
    }
    return [=](auto assets) {
        int res = pages->at(0)->getHeight() / 16;
        std::vector<std::unique_ptr<Texture>> textures;
        for (auto& page : *pages) {
            textures.emplace_back(Texture::from(page.get()));
        }
        assets->store(std::make_unique<Font>(std::move(textures), res, 4), name);
    };
}

assetload::postfunc assetload::layout(
    AssetsLoader*,
    const ResPaths* paths,
    const std::string& file,
    const std::string& name,
    const std::shared_ptr<AssetCfg>& config
) {
    return [=](auto assets) {
        try {
            auto cfg = std::dynamic_pointer_cast<LayoutCfg>(config);
            assets->store(UiDocument::read(cfg->env, name, file), name);
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
    const std::string& file,
    const std::string& name,
    const std::shared_ptr<AssetCfg>& config
) {
    auto cfg = std::dynamic_pointer_cast<SoundCfg>(config);
    bool keepPCM = cfg ? cfg->keepPCM : false;

    std::unique_ptr<audio::Sound> baseSound = nullptr;
    static std::vector<std::string> extensions {".ogg", ".wav"};
    std::string extension;
    for (size_t i = 0; i < extensions.size(); i++) {
        extension = extensions[i];
        // looking for 'sound_name' as base sound
        auto soundFile = paths->find(file+extension);
        if (fs::exists(soundFile)) {
            baseSound = audio::load_sound(soundFile, keepPCM);
            break;
        }
        // looking for 'sound_name_0' as base sound
        auto variantFile = paths->find(file+"_0"+extension);
        if (fs::exists(variantFile)) {
            baseSound = audio::load_sound(variantFile, keepPCM);
            break;
        }
    }
    if (baseSound == nullptr) {
        throw std::runtime_error("could not to find sound: " + file);
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
        assets->store(std::unique_ptr<audio::Sound>(sound), name);
    };
}

assetload::postfunc assetload::model(
    AssetsLoader* loader,
    const ResPaths* paths,
    const std::string& file,
    const std::string& name,
    const std::shared_ptr<AssetCfg>&
) {
    auto path = paths->find(file+".obj");
    auto text = files::read_string(path);
    try {
        auto model = obj::parse(path.u8string(), text).release();
        return [=](Assets* assets) {
            for (auto& mesh : model->meshes) {
                if (mesh.texture.find('$') == std::string::npos) {
                    auto filename = TEXTURES_FOLDER+"/"+mesh.texture;
                    loader->add(AssetType::TEXTURE, filename, mesh.texture, nullptr);
                }
            }
            assets->store(std::unique_ptr<model::Model>(model), name);
        };
    } catch (const parsing_error& err) {
        std::cerr << err.errorLog() << std::endl;
        throw;
    }
}

assetload::postfunc assetload::rig(
    AssetsLoader* loader,
    const ResPaths* paths,
    const std::string& file,
    const std::string& name,
    const std::shared_ptr<AssetCfg>&
) {
    auto path = paths->find(file+".json");
    auto text = files::read_string(path);
    try {
        auto rig = rigging::RigConfig::parse(text, path.u8string(), name).release();
        return [=](Assets* assets) {
            // TODO: add models loading
            assets->store(std::unique_ptr<rigging::RigConfig>(rig), name);
        };
    } catch (const parsing_error& err) {
        std::cerr << err.errorLog() << std::endl;
        throw;
    }
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

    const int extension = 2;

    frame.dstPos = glm::ivec2(region.u1 * dstWidth, region.v1 * dstHeight) - extension;
    frame.size = glm::ivec2(region.u2 * dstWidth, region.v2 * dstHeight) - frame.dstPos + extension;

    for (const auto& elem : frameList) {
        if (!srcAtlas->has(elem.first)) {
            std::cerr << "Unknown frame name: " << elem.first << std::endl;
            continue;
        }
        region = srcAtlas->get(elem.first);
        if (elem.second > 0) {
            frame.duration = static_cast<float>(elem.second) / 1000.0f;
        }
        frame.srcPos = glm::ivec2(region.u1 * srcWidth, srcHeight - region.v2 * srcHeight) - extension;
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
            if (!frameList.empty() && !contains(frameList, file.stem().u8string())) {
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
        assets->store(std::move(srcAtlas), atlasName + "/" + name + "_animation");
        assets->store(animation);
        return true;
    }
    return true;
}
