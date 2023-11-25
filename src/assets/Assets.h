#ifndef ASSETS_ASSETS_H_
#define ASSETS_ASSETS_H_

#include <string>
#include <unordered_map>

class ITexture;
class IShader;
class Font;
class Atlas;

class Assets {
	std::unordered_map<std::string, ITexture*> textures;
	std::unordered_map<std::string, IShader*> shaders;
	std::unordered_map<std::string, Font*> fonts;
	std::unordered_map<std::string, Atlas*> atlases;
public:
	~Assets();
	ITexture* getTexture(std::string name) const;
	void store(ITexture* texture, std::string name);

	IShader* getShader(std::string name) const;
	void store(IShader* shader, std::string name);

	Font* getFont(std::string name) const;
	void store(Font* font, std::string name);

	Atlas* getAtlas(std::string name) const;
	void store(Atlas* atlas, std::string name);
};

#endif /* ASSETS_ASSETS_H_ */
