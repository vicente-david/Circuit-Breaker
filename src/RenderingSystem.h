#pragma once 

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <memory>

#include "GameState.h"
#include "Text.h"
#include "Camera.h"
#include "ecs/System.h"

class RenderingSystem:public System {

public:
	RenderingSystem();
	void initializeShaders();
	void initializeText();

	void update(GameState& gamestate, std::string fps, Camera& c1);

	static std::shared_ptr<RenderingSystem> registerSystem(std::shared_ptr<Coordinator> &coord);


	unsigned int textVBO;
	unsigned int textVAO;

	std::map<char, Character> textFont;
	glm::mat4 textMat;

	GLFWwindow* window;
	std::unique_ptr<ShaderProgram> basicShader;
	std::unique_ptr<ShaderProgram> textProg;


private:
	
	
};


