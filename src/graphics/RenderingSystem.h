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
	void drawPhysxDebug(GameState &game, glm::mat4& view, glm::mat4& proj);
	void renderScene(GameState& game, GLuint& shaderID);
	void setTrackBounds(std::pair<glm::vec3, glm::vec3> trackBounds);

	static std::shared_ptr<RenderingSystem> registerSystem(std::shared_ptr<Coordinator> &coord);

	unsigned int SCR_WIDTH = 800, SCR_HEIGHT = 600;
	unsigned int SHADOW_WIDTH = SCR_WIDTH * 8, SHADOW_HEIGHT = SCR_HEIGHT * 8;
	unsigned int textVBO;
	unsigned int textVAO;
	unsigned int linesVBO;
	unsigned int depthFBO, depthMap;

	std::map<char, Character> textFont;
	glm::mat4 textMat;

	GLFWwindow* window;
	std::unique_ptr<ShaderProgram> basicShader;
	std::unique_ptr<ShaderProgram> textProg;
	std::unique_ptr<ShaderProgram> solidColour;

	std::unique_ptr<ShaderProgram> shadowShader;


private:
	std::pair<glm::vec3, glm::vec3> bounds;
	
	
};


