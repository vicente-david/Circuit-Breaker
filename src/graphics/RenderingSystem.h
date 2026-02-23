#pragma once 

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <memory>

#include "GameState.h"
#include "Text.h"
#include "Camera.h"
#include "ecs/System.h"
#include "graphics/CameraSystem.h"

class RenderingSystem:public System {

public:
	RenderingSystem();
	void initializeShaders();
	void initializeText();
	void initShadowMap();

	void update(GameState& gamestate, std::string fps, std::shared_ptr<CameraSystem> cameraSystem);
	void renderScene(GameState& game, GLuint& shaderID);

	static std::shared_ptr<RenderingSystem> registerSystem(std::shared_ptr<Coordinator> &coord);

	unsigned int SCR_WIDTH = 800, SCR_HEIGHT = 600;
	unsigned int textVBO;
	unsigned int textVAO;
	unsigned int depthFBO, depthMap;

	std::map<char, Character> textFont;
	glm::mat4 textMat;

	GLFWwindow* window;
	std::unique_ptr<ShaderProgram> basicShader;
	std::unique_ptr<ShaderProgram> textProg;
	std::unique_ptr<ShaderProgram> shadowShader;


private:
	
	
};


