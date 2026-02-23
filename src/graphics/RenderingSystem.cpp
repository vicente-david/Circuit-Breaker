#include "RenderingSystem.h"
#include "GameState.h"
#include "debugUtils/Logger.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <memory>
#include "CameraSystem.h"
#include "graphics/CameraComp.h"
#include "graphics/Model.h"

void framebuffer_size_callback(GLFWwindow *window, int width,
							   int height); // TODO: move this

RenderingSystem::RenderingSystem() : textVBO(1), textVAO(1) {
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
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
	glViewport(0, 0, width, height);
}

void RenderingSystem::initializeShaders() {
	dbug::log("REND", 0, "loading shaders");
	// Create shader program
	basicShader = std::make_unique<ShaderProgram>("shaders/basic.vert",
												  "shaders/basic.frag");
	shadowShader = std::make_unique<ShaderProgram>("shaders/shadow.vert",
												"shaders/shadow.frag");
	textProg = std::make_unique<ShaderProgram>("shaders/testText.vert",
											   "shaders/testText.frag");
	textFont = initFont("assets/miamanueva.ttf");
	textMat = glm::ortho(0.0f, static_cast<float>(1440), 0.0f,
						 static_cast<float>(1440));
	textProg->use();
	glUniformMatrix4fv(glGetUniformLocation(textProg->id, "projection"), 1,
					   GL_FALSE, glm::value_ptr(textMat));
	initShadowMap();
}

void RenderingSystem::initShadowMap() {

	glGenFramebuffers(1, &depthFBO);

	// 2D texture for depth buffer
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		800, 600, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE); // no colour data to render
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void RenderingSystem::initializeText() {
	// Initialize and bind VAO
	glGenVertexArrays(1, &textVAO);
	glBindVertexArray(textVAO);
	// Generate VBO
	glGenBuffers(1, &textVBO);
	// bind VBO
	glBindBuffer(GL_ARRAY_BUFFER, textVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void RenderingSystem::update(GameState &game, std::string fps, std::shared_ptr<CameraSystem> camSystem) {
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.1f, 0.1f, 0.1f, 0.1f);
	// Render pass 1: depth to texture
	float near_plane = 1.0f, far_plane = 7.5f;
	glm::mat4 lightProj = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
	glm::mat4 lightView = glm::lookAt(glm::vec3(0.3, 3.0, 1.0), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 lightSpaceMat = lightProj * lightView;
	
	shadowShader->use();
	unsigned int lightSpaceLoc = glGetUniformLocation(shadowShader->id, "lightSpaceMat");
	glUniformMatrix4fv(lightSpaceLoc, 1, GL_FALSE, glm::value_ptr(lightSpaceMat));

	glViewport(0, 0, 800, 600);
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
	// view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
	view = c1->GetViewMatrix();
	glm::mat4 proj;
	proj = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

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

	// render text
	textProg->use();
	RenderText(textProg->id, textVAO, textVBO, "FPS: " + fps, 10.f, 1380.f,
			   1.0f, glm::vec3(1.0f), textFont);

	glfwPollEvents();
	glfwSwapBuffers(window);
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

std::shared_ptr<RenderingSystem>
RenderingSystem::registerSystem(std::shared_ptr<Coordinator> &coord) {
	// register system
	auto system = coord->registerSystem<RenderingSystem>();
	// create system signture (what components this system needs)
	Signature sig;
	// sig.set(coord->getComponentType<Transform>());
	sig.set(coord->getComponentType<Transform>());
	sig.set(coord->getComponentType<Model>());
	coord->setSystemSignature<RenderingSystem>(sig);

	return system;
}
