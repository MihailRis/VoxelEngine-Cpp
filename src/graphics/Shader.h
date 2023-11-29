#ifndef GRAPHICS_SHADER_H_
#define GRAPHICS_SHADER_H_

#include <string>
#include <glm/glm.hpp>

#include "../graphics-base/IShader.h"

class GLSLExtension;

class Shader : public IShader {
public:
	static GLSLExtension* preprocessor;
	unsigned int id;

	Shader(unsigned int id);
	~Shader() override;

	void use() override;
	void uniformMatrix(std::string name, glm::mat4 matrix) override;
	void uniform1i(std::string name, int x) override;
	void uniform1f(std::string name, float x) override;
	void uniform2f(std::string name, float x, float y) override;
	void uniform2f(std::string name, glm::vec2 xy) override;
	void uniform3f(std::string name, float x, float y, float z) override;
	void uniform3f(std::string name, glm::vec3 xyz) override;

	static Shader* loadShader(std::string vertexFile, std::string fragmentFile);
};

#endif /* GRAPHICS_SHADER_H_ */
