#pragma once
#include <glad/gl.h>
#include <string>
#include "Shader.h"

/*
Link shaders to create shader program
*/
class ShaderProgram {
public:
	ShaderProgram(const std::string& vertexPath, const std::string& fragmentPath);
	ShaderProgram(const std::string& vertexPath, const std::string& fragmentPath, const std::string& geomPath);

	void use() const {
		glUseProgram(id);
	}

	// Allow shader to access private members for linking shader program
	void friend attachShader(ShaderProgram& prog, Shader& shad);

	void friend shaderCleanup(Shader& vert, Shader& frag);
	void friend shaderCleanup(Shader& vert, Shader& frag, Shader& geom);
	

	GLuint id;

private:
	

	Shader vertex;
	Shader fragment;
	Shader geometry;

	bool checkLink();
};
