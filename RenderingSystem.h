#include <memory>
#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "Text.h"
#include "Model.h"

class RenderingSystem {

public:
	RenderingSystem();
	unsigned int initVAO(Vertex* vertices, int size);
	void initializeShaders();
	void initializeText();

	unsigned int textVBO;
	unsigned int textVAO;

	std::map<char, Character> textFont;
	glm::mat4 textMat;

	GLFWwindow* window;
	std::unique_ptr<ShaderProgram> basicShader;
	std::unique_ptr<ShaderProgram> textProg;

private:
	
	
};


