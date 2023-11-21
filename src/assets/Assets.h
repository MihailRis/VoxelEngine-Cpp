#ifndef ASSETS_ASSETS_H_
#define ASSETS_ASSETS_H_

#include <string>
#include <unordered_map>

class Texture;
class Shader;
class Font;
class Atlas;

class Assets {
	std::unordered_map<std::string, Texture*> textures;
	std::unordered_map<std::string, Shader*> shaders;
	std::unordered_map<std::string, Font*> fonts;
	std::unordered_map<std::string, Atlas*> atlases;
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
};

#endif /* ASSETS_ASSETS_H_ */
