#include <memory>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include "ShaderProgram.h"

class RenderingSystem {

public:
	RenderingSystem();
	void initializeRenderer();
	void initializeShaders(float* vertices, int size);

	unsigned int VAO;
	// might need multiple vaos and a bind method, or even an id system to determine which vao to bind
	unsigned int VBO;

	GLFWwindow* window;
	std::unique_ptr<ShaderProgram> shaderProg;
	// need a way to store multiple shaders
	std::unique_ptr<ShaderProgram> textProg;

private:
	
	
};


