#include <memory>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include "ShaderProgram.h"

class RenderingSystem {

public:
	RenderingSystem();

	unsigned int VAO;
	unsigned int VBO;

	GLFWwindow* window;
	std::unique_ptr<ShaderProgram> shaderProg;

private:
	
	
};


