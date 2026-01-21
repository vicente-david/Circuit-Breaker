#include <glad/gl.h>
#include <string>
#include "Shader.h"

/*
Link shaders to create shader program
*/
class ShaderProgram {
public:
	ShaderProgram(const std::string& vertexPath, const std::string& fragmentPath);

	void use() const {
		glUseProgram(id);
	}

	// Allow shader to access private members for linking shader program
	void friend attachShader(ShaderProgram& prog, Shader& shad);

	void friend shaderCleanup(Shader& vert, Shader& frag);

private:
	GLuint id;

	Shader vertex;
	Shader fragment;

	bool checkLink();
};