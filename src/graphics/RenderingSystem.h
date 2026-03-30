#pragma once 
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <memory>
#include "ShaderProgram.h"
#include "GameState.h"
#include "Camera.h"
#include "ecs/System.h"
#include "graphics/CameraSystem.h"

// first setup everything (initilaize shaders and test and shadow map)
// each frame, rendering system knows which "rendering passes" it needs to do
// will just be a vector of function pointers, and it will call each one sequentially
// gamestate will ultimately modify which things to render (i.e update the vector of function ptrs accordingly)

// rendering system will always be active while the game is running, so theoretically there shouldn't be
// any memory issues

class RenderingSystem:public System {

public:
	RenderingSystem();
	void initializeShaders();
	void initializeLines();
	void initShadowMap();

	void update(GameState& gamestate, std::string fps, std::shared_ptr<CameraSystem> cameraSystem);
	void renderShadows(GameState& gamestate, std::string& fps, std::shared_ptr<CameraSystem> camSystem); // Render pass 1 


	void drawPhysxDebug(GameState &game, glm::mat4& view, glm::mat4& proj);
	void renderScene(GameState& game, GLuint& shaderID);
	void setTrackBounds(std::pair<glm::vec3, glm::vec3> trackBounds);

	


	static std::shared_ptr<RenderingSystem> registerSystem(std::shared_ptr<Coordinator> &coord);

	int SCR_WIDTH = 1200, SCR_HEIGHT = 800;
	unsigned int SHADOW_WIDTH = SCR_WIDTH * 10, SHADOW_HEIGHT = SCR_HEIGHT * 10;

	unsigned int linesVBO, linesVAO;
	unsigned int depthFBO, depthMap;
	unsigned int matricesUBO;

	

	GLFWwindow* window;
	std::unique_ptr<ShaderProgram> basicShader;
	
	std::unique_ptr<ShaderProgram> solidColour;

	std::unique_ptr<ShaderProgram> shadowShader;

	// ah yes my favourite type in c++
	// let's break this down
	// x y z 
	// x = void (the return type)
	// y = (RenderingSystem::*) this is a pointer to a member function
		// think of normal types like int x;
		// then we have int* x;  (pointer to an integer)
		// instead we have RenderingSystem::* x;  
		// RenderingSystem::*  is a type, which again is just a pointer to a member function   
	// z = (GameState& game, std::string& fps, std::shared_ptr<CameraSystem> camSystem) this is just the arguments of the function
	// z solidifies which member function signature we can throw in the vector 
	// i.e we can't put RenderingSystem::initializeShaders in this vector

	// in essence
	// return type (function) (args)
	std::vector<void (RenderingSystem::*) (GameState& game, std::string& fps, std::shared_ptr<CameraSystem> camSystem)> renderPasses;

	// now how do we use it?
	// simple
	// int* x, you store it with &y  (& address of)
	// so we just do
	// renderPasses.push_back(RenderingSystem::&renderShadows)
	// to access in RenderingSystem (this->*renderPasses[i])(game, fps, camSystem)
	// LMAO what is C++

private:
	std::pair<glm::vec3, glm::vec3> bounds;

	// some shadow map stuff
	std::vector<glm::vec4> getFrustumCorners(const glm::mat4& proj, const glm::mat4& view);
	glm::mat4 lightViewProjMat(const float nearPlane, const float farPlane, glm::mat4 view);
	std::vector<glm::mat4> getLightSpaceMatrices(glm::mat4 view);
	float nearPlane = 0.1f;
	float farPlane = 200.0f;
	std::vector<float> shadowCascadeLevels{ farPlane / 50.0f, farPlane / 25.0f, farPlane / 10.0f, farPlane / 2.0f };
	
};


