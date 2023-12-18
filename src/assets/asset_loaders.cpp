#include "asset_loaders.h"

#include <iostream>
#include <filesystem>
#include "Assets.h"
#include "../files/files.h"
#include "../files/engine_paths.h"
#include "../coders/png.h"
#include "../graphics/Shader.h"
#include "../graphics/Texture.h"
#include "../graphics/ImageData.h"
#include "../graphics/Atlas.h"
#include "../graphics/Font.h"


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

bool assetload::atlas(Assets* assets, 
                      const ResPaths* paths,
                      const std::string directory, 
                      const std::string name) {
	AtlasBuilder builder;
	for (const auto& file : paths->listdir(directory)) {
		// png is only supported format
		if (file.extension() != ".png")
			continue;
		std::string name = file.stem().string();
		// skip duplicates
		if (builder.has(name)) {
			continue;
		}
		std::unique_ptr<ImageData> image (png::load_image(file.string()));
		if (image == nullptr) {
			std::cerr << "could not to load " << file.string() << std::endl;
			continue;
		}
		image->fixAlphaColor();
		builder.add(name, image.release());
	}
	Atlas* atlas = builder.build(2);
	assets->store(atlas, name);
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
