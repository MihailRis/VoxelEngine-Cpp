#ifndef SRC_ASSETS_H_
#define SRC_ASSETS_H_

#include <string>
#include <unordered_map>

class Texture;
class Shader;

class Assets {
	std::unordered_map<std::string, Texture*> textures;
	std::unordered_map<std::string, Shader*> shaders;
public:
	~Assets();
	Texture* getTexture(std::string name);
	void store(Texture* texture, std::string name);

	Shader* getShader(std::string name);
	void store(Shader* shader, std::string name);
};

#endif /* SRC_ASSETS_H_ */
