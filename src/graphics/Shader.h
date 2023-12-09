#ifndef GRAPHICS_SHADER_H_
#define GRAPHICS_SHADER_H_

#include <string>
#include <glm/glm.hpp>

class GLSLExtension;

class Shader {
public:
	static GLSLExtension* preprocessor;
	unsigned int id;

	Shader(unsigned int id);
	~Shader();

	void use();
	void uniformMatrix(const std::string_view& name, const glm::mat4& matrix);
	void uniform1i(const std::string_view& name, int x);
	void uniform1f(const std::string_view& name, float x);
	void uniform2f(const std::string_view& name, float x, float y);
	void uniform2f(const std::string_view& name, glm::vec2 xy);
	void uniform3f(const std::string_view& name, float x, float y, float z);
	void uniform3f(const std::string_view& name, const glm::vec3& xyz);

	static Shader* loadShader(const std::string& vertexFile, const std::string& fragmentFile,
							  std::string& vertexSource, std::string& fragmentSource);
};

#endif /* GRAPHICS_SHADER_H_ */
