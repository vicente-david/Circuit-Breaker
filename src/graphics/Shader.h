#pragma once
#include <glad/gl.h>
#include <string>

class ShaderProgram;

/*
Handle setup of a shader (vertex or fragment)
*/
class Shader {

public:
	Shader(GLenum type, const std::string& path);

	// Allow shaderProgram to access private members for linking shader program
	// https://stackoverflow.com/questions/17434/when-should-you-use-friend-in-c
	void friend attachShader(ShaderProgram& prog, Shader& shad);

	void friend shaderCleanup(Shader& vert, Shader& frag);

private:
	GLenum type;
	GLuint id;
	std::string path;

	bool compile();
};