#include "assetload_funcs.h"

#include <iostream>
#include <filesystem>
#include "Assets.h"
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

namespace fs = std::filesystem;

bool assetload::texture(Assets* assets,
                        const ResPaths* paths,
                        const std::string filename, 
                        const std::string name) {
	Texture* texture = png::load_texture(paths->find(filename).string());
	if (texture == nullptr) {
		std::cerr << "failed to load texture '" << name << "'" << std::endl;
		return false;
	}
	assets->store(texture, name);
	return true;
}

bool assetload::shader(Assets* assets,
                       const ResPaths* paths,
                       const std::string filename, 
                       const std::string name) {
	fs::path vertexFile = paths->find(filename+".glslv");
	fs::path fragmentFile = paths->find(filename+".glslf");

	std::string vertexSource = files::read_string(vertexFile);
	std::string fragmentSource = files::read_string(fragmentFile);

	Shader* shader = Shader::loadShader(
		vertexFile.string(),
		fragmentFile.string(),
		vertexSource, fragmentSource);

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

bool assetload::atlas(Assets* assets, 
                      const ResPaths* paths,
                      const std::string directory, 
                      const std::string name) {
	AtlasBuilder builder;
	for (const auto& file : paths->listdir(directory)) {
		if (!appendAtlas(builder, file)) continue;
	}
	Atlas* atlas = builder.build(2);
	assets->store(atlas, name);
	for (const auto& file : builder.getNames()) {
		assetload::animation(assets, paths, "textures", file, atlas);
	}
	return true;
}

bool assetload::font(Assets* assets, 
                     const ResPaths* paths,
                     const std::string filename, 
                     const std::string name) {
	std::vector<Texture*> pages;
	for (size_t i = 0; i <= 4; i++) {
        std::string name = filename + "_" + std::to_string(i) + ".png"; 
        name = paths->find(name).string();
		Texture* texture = png::load_texture(name);
		if (texture == nullptr) {
			std::cerr << "failed to load bitmap font '" << name;
            std::cerr << "' (missing page " << std::to_string(i) << ")";
            std::cerr << std::endl;
			return false;
		}
		pages.push_back(texture);
	}
	Font* font = new Font(pages, pages[0]->height / 16);
	assets->store(font, name);
	return true;
}

bool assetload::animation(Assets* assets, 
						const ResPaths* paths, 
						const std::string directory, 
						const std::string name,
						Atlas* dstAtlas) {
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

			Frame temp;
			float frameDuration = DEFAULT_FRAME_DURATION;
			std::string frameName;

			if (frameArr) {
				for (size_t i = 0; i < frameArr->size(); i++) {
					auto currentFrame = frameArr->list(i);

					frameName = currentFrame->str(0);
					if (currentFrame->size() > 1) frameDuration = static_cast<float>(currentFrame->integer(1)) / 1000;

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

		Atlas* srcAtlas = builder.build(2);

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

		assets->store(srcAtlas, name + "_animation");
		assets->store(animation);

		return true;
	}
	return true;
}
