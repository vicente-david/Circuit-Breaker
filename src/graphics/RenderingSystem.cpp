#pragma once
#include "RenderingSystem.h"
#include "CameraSystem.h"
#include "GameState.h"
#include "debugUtils/Logger.h"
#include "debugUtils/Panel.h"
#include "graphics/Model.h"
#include <GL/gl.h>
#include <glm/fwd.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <memory>

RenderingSystem::RenderingSystem(){
	// Instantiate GLFW window
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Create window object
	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Circuit Breaker", NULL, NULL);
	if (window == NULL) {
		glfwTerminate();
		std::cout << "Window creation failed." << std::endl;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLAD
	if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) {
		std::cout << "GLAD initialization failed." << std::endl;
	}

	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

}


void RenderingSystem::initializeShaders() {
	dbug::log("REND", 0, "loading shaders");

	// Create shader program
	basicShader = std::make_unique<ShaderProgram>("shaders/basic.vert",
												  "shaders/basic.frag");
	shadowShader = std::make_unique<ShaderProgram>("shaders/shadow.vert",
												"shaders/shadow.frag");
	solidColour = std::make_unique<ShaderProgram>("shaders/lines.vert",
												  "shaders/lines.frag");

	initShadowMap();
}

void RenderingSystem::initShadowMap() {

	glGenFramebuffers(1, &depthFBO);

	// 2D texture for depth buffer
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderCol[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderCol);
	glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE); // no colour data to render
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void RenderingSystem::initializeLines() {

	// lines debug VAO/VBO setup
	glGenVertexArrays(1, &linesVAO);
	glBindVertexArray(linesVAO);
	glGenBuffers(1, &linesVBO);
	glBindBuffer(GL_ARRAY_BUFFER, linesVBO);
	glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

// TODO: split the rendering passes
void RenderingSystem::renderShadows(GameState& game, std::string& fps, std::shared_ptr<CameraSystem> camSystem) {
	// Render pass 1: depth to texture
	float near_plane = -70.f, far_plane = 25.0f;
	glm::mat4 lightProj = glm::ortho(bounds.first.x - 50.f, bounds.second.x + 50.f, bounds.first.z - 50.f, bounds.second.z + 50.f, near_plane, far_plane);
	glm::mat4 lightView = glm::lookAt(glm::vec3(0.0f, 1.0f, 0.1f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 lightSpaceMat = lightProj * lightView;

	shadowShader->use();
	unsigned int lightSpaceLoc = glGetUniformLocation(shadowShader->id, "lightSpaceMat");
	glUniformMatrix4fv(lightSpaceLoc, 1, GL_FALSE, glm::value_ptr(lightSpaceMat));

	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
	glClear(GL_DEPTH_BUFFER_BIT);

	renderScene(game, shadowShader->id);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	// Render pass 2: render scene as normal

	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	basicShader->use();
	auto c1 = camSystem->cameras[0];

	glm::mat4 view = glm::mat4(1.0f);
	view = c1->GetViewMatrix();
	glm::mat4 proj;
	proj = glm::perspective(glm::radians(50.0f), static_cast<float>(SCR_WIDTH) / static_cast<float>(SCR_HEIGHT), 0.1f, 200.0f);

	unsigned int viewLoc = glGetUniformLocation(basicShader->id, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	unsigned int projLoc = glGetUniformLocation(basicShader->id, "projection");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));
	lightSpaceLoc = glGetUniformLocation(basicShader->id, "lightSpaceMat");
	glUniformMatrix4fv(lightSpaceLoc, 1, GL_FALSE, glm::value_ptr(lightSpaceMat));
	glUniform1i(glGetUniformLocation(basicShader->id, "shadowMap"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthMap);

	renderScene(game, basicShader->id);

	if (dbugPanel::tuning::physicsShapes) {
		drawPhysxDebug(game, view, proj);
	}

}

void RenderingSystem::update(GameState &game, std::string fps, std::shared_ptr<CameraSystem> camSystem) {
	glfwGetWindowSize(window, &SCR_WIDTH, &SCR_HEIGHT);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.1f, 0.1f, 0.1f, 0.1f);
	
	// render shadows + scene
	//renderShadows(game, fps ,camSystem);
	

	for (auto renderPass : renderPasses) {
		(this->*renderPass)(game, fps, camSystem);
	}

	
	// ui is rendered next, just in the upper level
	
	dbugPanel::render();

}

void RenderingSystem::renderScene(GameState& game, GLuint& shaderID) {
	// draw every entities model at the location of it's transform
	for (auto& entity : entities) {
		// dbug::log("REND",0, "Drawing entity %d", entity);

		Model& model = game.coordinator->getComponent<Model>(entity);
		Transform& transform =
			game.coordinator->getComponent<Transform>(entity);

		glm::mat4 modelTransform = glm::mat4(1.0f);
		modelTransform = glm::translate(modelTransform, transform.pos);
		modelTransform *= glm::toMat4(transform.rot);

		// use transformations
		unsigned int modelLoc = glGetUniformLocation(shaderID, "model");
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE,
			glm::value_ptr(modelTransform));

		model.Draw(shaderID);
	}
	
}

// Needs to be set once for track
void RenderingSystem::setTrackBounds(std::pair<glm::vec3, glm::vec3> trackBounds) {
	this->bounds = trackBounds;
}

void RenderingSystem::drawPhysxDebug(GameState &game, glm::mat4 &view,
									 glm::mat4 &proj) {
	// draw physx geometry render
	const PxRenderBuffer &physXRBuffer =
		game.physics->gScene->getRenderBuffer();
	
	std::vector<glm::vec3> lines;
	// glm::vec3 lines[6] = {
	// 	glm::vec3(-0.5,0.5,0), glm::vec3(0.5,0.5,0),
	// 	glm::vec3(0,0,0),glm::vec3(-10,1,1),
	// 	glm::vec3(10,-0.5,1), glm::vec3(0,0,0)
	// };

	//printf("nlines:%d\n", physXRBuffer.getNbLines());
	for (PxU32 i = 0; i < physXRBuffer.getNbLines(); i++) {
		auto line = physXRBuffer.getLines()[i];
		glm::vec3 p1(line.pos0.x, line.pos0.y, line.pos0.z);
		glm::vec3 p2(line.pos1.x, line.pos1.y, line.pos1.z);
		lines.push_back(p1);
		lines.push_back(p2);

	}
	// std::cout << "lines size: " << lines.size() << std::endl;
	// bind shader and stuff
	solidColour->use();
	glBindVertexArray(linesVAO);
	glBindBuffer(GL_ARRAY_BUFFER, linesVBO);
	
	glBufferData(GL_ARRAY_BUFFER, lines.size() * sizeof(glm::vec3), lines.data(), GL_DYNAMIC_DRAW);

	// set uniforms
	unsigned int uniLoc = glGetUniformLocation(solidColour->id, "model");
	glUniformMatrix4fv(uniLoc, 1, GL_FALSE, glm::value_ptr(glm::mat4(1)));
	uniLoc = glGetUniformLocation(solidColour->id, "view");
	glUniformMatrix4fv(uniLoc, 1, GL_FALSE, glm::value_ptr(view));
	uniLoc = glGetUniformLocation(solidColour->id, "projection");
	glUniformMatrix4fv(uniLoc, 1, GL_FALSE, glm::value_ptr(proj));

	// draw the things
	glDrawArrays(GL_LINES, 0, lines.size());
	glBindVertexArray(0);

}

std::shared_ptr<RenderingSystem>
RenderingSystem::registerSystem(std::shared_ptr<Coordinator> &coord) {
	// register system
	auto system = coord->registerSystem<RenderingSystem>();
	// create system signture (what components this system needs)
	Signature sig;
	sig.set(coord->getComponentType<Transform>());
	sig.set(coord->getComponentType<Model>());
	coord->setSystemSignature<RenderingSystem>(sig);

	return system;
}
