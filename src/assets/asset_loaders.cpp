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

using std::string;
using std::vector;
using std::unique_ptr;
using std::filesystem::path;

namespace fs = std::filesystem;

bool assetload::texture(Assets* assets, 
                        const ResPaths* paths,
                        const string filename, 
                        const string name) {
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
                       const string filename, 
                       const string name) {
    path vertexFile = paths->find(filename+".glslv");
    path fragmentFile = paths->find(filename+".glslf");
    
    string vertexSource = files::read_string(vertexFile);
    string fragmentSource = files::read_string(fragmentFile);

	Shader* shader = Shader::loadShader(vertexFile.string(), fragmentFile.string(),
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
                      const string directory, 
                      const string name) {
	AtlasBuilder builder;
	for (const auto& file : paths->listdir(directory)) {
		if (file.extension() == ".png") {
			string name = file.stem().string();
            if (builder.has(name)) {
                continue; // skip duplicates
            }
			std::unique_ptr<ImageData> image (png::load_image(file.string()));//but what if load_image return nullptr?
			image->fixAlphaColor();
			builder.add(name, image.release());
		}
	}
	Atlas* atlas = builder.build(2);
	assets->store(atlas, name);
	return true;
}

bool assetload::font(Assets* assets, 
                     const ResPaths* paths,
                     const string filename, 
                     const string name) {
	vector<Texture*> pages;
	for (size_t i = 0; i <= 4; i++) {
        string name = filename + "_" + std::to_string(i) + ".png"; 
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
