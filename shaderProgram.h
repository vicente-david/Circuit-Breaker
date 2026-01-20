#include <glad/gl.h>
#include <string>
#include "Shader.h"

/*
Link shaders to create shader program
*/
class shaderProgram {
public:
	shaderProgram(const std::string& vertexPath, const std::string& fragmentPath);

	void use() const {
		glUseProgram(id);
	}

	// Allow shader to access private members for linking shader program
	void friend attachShader(shaderProgram& prog, shader& shad);

	// TODO: RAII cleanup (replace shaderCleanup)
	void friend shaderCleanup(shaderProgram& prog, shader& vert, shader& frag);

private:
	GLuint id;

	shader vertex;
	shader fragment;

	bool link();
};