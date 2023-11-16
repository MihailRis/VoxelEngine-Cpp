#ifndef GRAPHICS_SHADER_H_
#define GRAPHICS_SHADER_H_

#include <string>
#include <glm/glm.hpp>

class Shader {
public:
	unsigned int id;

	Shader(unsigned int id);
	~Shader();

	void use();
	void uniformMatrix(std::string name, glm::mat4 matrix);
	void uniform1i(std::string name, int x);
	void uniform1f(std::string name, float x);
	void uniform2f(std::string name, float x, float y);
	void uniform2f(std::string name, glm::vec2 xy);
	void uniform3f(std::string name, float x, float y, float z);
	void uniform3f(std::string name, glm::vec3 xyz);
};

extern Shader* load_shader(std::string vertexFile, std::string fragmentFile);

#endif /* GRAPHICS_SHADER_H_ */
