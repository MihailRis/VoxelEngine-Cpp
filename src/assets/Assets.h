#ifndef ASSETS_ASSETS_H_
#define ASSETS_ASSETS_H_

#include "../graphics/TextureAnimation.h"

#include <string>
#include <memory>
#include <unordered_map>
#include <vector>

class Texture;
class Shader;
class Font;
class Atlas;

class Assets {
	std::unordered_map<std::string, std::shared_ptr<Texture>> textures;
	std::unordered_map<std::string, std::shared_ptr<Shader>> shaders;
	std::unordered_map<std::string, std::shared_ptr<Font>> fonts;
	std::unordered_map<std::string, std::shared_ptr<Atlas>> atlases;
	std::vector<TextureAnimation> animations;
public:
	~Assets();
	Texture* getTexture(std::string name) const;
	void store(Texture* texture, std::string name);

	Shader* getShader(std::string name) const;
	void store(Shader* shader, std::string name);

	Font* getFont(std::string name) const;
	void store(Font* font, std::string name);

	Atlas* getAtlas(std::string name) const;
	void store(Atlas* atlas, std::string name);

	const std::vector<TextureAnimation>& getAnimations();
	void store(const TextureAnimation& animation);

    void extend(const Assets& assets);
};

#endif /* ASSETS_ASSETS_H_ */
