#ifndef ASSETS_ASSETS_H_
#define ASSETS_ASSETS_H_

#include <string>
#include <memory>
#include <unordered_map>

class ITexture;
class IShader;
class Font;
class Atlas;

class Assets {
	std::unordered_map<std::string, std::shared_ptr<ITexture>> textures;
	std::unordered_map<std::string, std::shared_ptr<IShader>> shaders;
	std::unordered_map<std::string, std::shared_ptr<Font>> fonts;
	std::unordered_map<std::string, std::shared_ptr<Atlas>> atlases;
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

    void extend(const Assets& assets);
};

#endif /* ASSETS_ASSETS_H_ */
