#include <memory>
#include <vector>
#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "Text.h"
#include "Model.h"
#include "Entity1.h"
#include "Camera.h"

class RenderingSystem {

public:
	RenderingSystem();
	unsigned int initVAO(Vertex* vertices, int vertSize, unsigned int* indices, int indSize);
	void initializeShaders();
	void initializeText();

	void update(std::vector<Entity1> entities, unsigned int VAO, std::string fps, Camera& c1); //temporarily adding VAO here

	unsigned int textVBO;
	unsigned int textVAO;

	std::map<char, Character> textFont;
	glm::mat4 textMat;

	GLFWwindow* window;
	std::unique_ptr<ShaderProgram> basicShader;
	std::unique_ptr<ShaderProgram> textProg;


private:
	
	
};


