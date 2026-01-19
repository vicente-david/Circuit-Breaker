#include <glad/gl.h>
#include <string>

class shaderProgram;

class shader {

public:
	shader(GLenum type, const std::string& path);

	// Allow shaderProgram to access private members for linking shader program
	// https://stackoverflow.com/questions/17434/when-should-you-use-friend-in-c
	void friend attachShader(shaderProgram& prog, shader& shad);

	// TODO: RAII cleanup (replace shaderCleanup)
	void friend shaderCleanup(shaderProgram& prog, shader& vert, shader& frag);

private:
	GLenum type;
	GLuint id;
	std::string path;

	bool compile();
};