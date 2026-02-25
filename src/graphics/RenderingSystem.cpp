#include "RenderingSystem.h"
#include "CameraSystem.h"
#include "GameState.h"
#include "InputSystem.h"
#include "debugUtils/Logger.h"
#include "graphics/CameraComp.h"
#include "graphics/Mesh.h"
#include "graphics/Model.h"
#include <GL/gl.h>
#include <glm/fwd.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <memory>

void framebuffer_size_callback(GLFWwindow *window, int width,
							   int height); // TODO: move this

RenderingSystem::RenderingSystem() : textVBO(1), textVAO(1) {
	// Instantiate GLFW window
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Create window object
	window = glfwCreateWindow(800, 600, "Circuit Breaker", NULL, NULL);
	if (window == NULL) {
		glfwTerminate();
		std::cout << "Window creation failed." << std::endl;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLAD
	if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) {
		std::cout << "GLAD initialization failed." << std::endl;
	}

	glViewport(0, 0, 800, 600);
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
	textProg = std::make_unique<ShaderProgram>("shaders/testText.vert",
											   "shaders/testText.frag");
	solidColour = std::make_unique<ShaderProgram>("shaders/lines.vert",
												  "shaders/lines.frag");
	textFont = initFont("assets/miamanueva.ttf");
	textMat = glm::ortho(0.0f, static_cast<float>(1440), 0.0f,
						 static_cast<float>(1440));
	textProg->use();
	glUniformMatrix4fv(glGetUniformLocation(textProg->id, "projection"), 1,
					   GL_FALSE, glm::value_ptr(textMat));
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

	// lines vbo
	glGenBuffers(1, &linesVBO);
	glBindBuffer(GL_ARRAY_BUFFER, linesVBO);
	glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), 0);
}

void RenderingSystem::update(GameState &game, std::string fps,
							 std::shared_ptr<CameraSystem> camSystem) {

	auto c1 = camSystem->cameras[0];

	glm::mat4 view = glm::mat4(1.0f);
	// view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
	view = c1->GetViewMatrix();
	glm::mat4 proj;
	proj = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// for some reason drawing this before entities wont work
	// drawPhysxDebug(game, view, proj);

	// draw entities with basic shader
	basicShader->use();
	unsigned int viewLoc = glGetUniformLocation(basicShader->id, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	unsigned int projLoc = glGetUniformLocation(basicShader->id, "projection");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));

	// draw every entities model at the location of it's transform
	for (auto &entity : entities) {
		// dbug::log("REND",0, "Drawing entity %d", entity);

		Model &model = game.coordinator->getComponent<Model>(entity);
		Transform &transform =
			game.coordinator->getComponent<Transform>(entity);

		glm::mat4 modelTransform = glm::mat4(1.0f);
		modelTransform = glm::translate(modelTransform, transform.pos);
		modelTransform *= glm::toMat4(transform.rot);

		// use transformations
		unsigned int modelLoc = glGetUniformLocation(basicShader->id, "model");
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE,
						   glm::value_ptr(modelTransform));

		model.Draw(basicShader->id);
	}
	
	// this makes the last entity get drawn wrong idk why
	drawPhysxDebug(game, view, proj);

	// render text
	textProg->use();
	RenderText(textProg->id, textVAO, textVBO, "FPS: " + fps, 10.f, 1380.f,
			   1.0f, glm::vec3(1.0f), textFont);

	glfwPollEvents();
	glfwSwapBuffers(window);
}

void RenderingSystem::drawPhysxDebug(GameState &game, glm::mat4 &view,
									 glm::mat4 &proj) {
	// draw physx geometry render
	const PxRenderBuffer &physXRBuffer =
		game.physics->gScene->getRenderBuffer();

	glm::vec3 lines[physXRBuffer.getNbLines() * 2];
	// glm::vec3 lines[6] = {
	// 	glm::vec3(-0.5,0.5,0), glm::vec3(0.5,0.5,0),
	// 	glm::vec3(0,0,0),glm::vec3(-10,1,1), 
	// 	glm::vec3(10,-0.5,1), glm::vec3(0,0,0)
	// };
	printf("nlines:%d\n", physXRBuffer.getNbLines());
	for (PxU32 i = 0; i < physXRBuffer.getNbLines(); i++) {
		int arrIdx = i * 2;
		auto line = physXRBuffer.getLines()[i];
		glm::vec3 p1(line.pos0.x, line.pos0.y, line.pos0.z);
		glm::vec3 p2(line.pos1.x, line.pos1.y, line.pos1.z);
		lines[arrIdx] = p1;
		lines[arrIdx + 1] = p2;
		printf("line: [%f,%f, %f] [%f, %f, %f] \n", p1.x, p1.y, p1.z, p2.x,
			   p2.y, p2.z);
	}
	// bind shader and stuff
	solidColour->use();
	glBindBuffer(GL_ARRAY_BUFFER, linesVBO);
	// it wont draw properly if this isn't set each frame
	// it will break the last drawn entity for some reason though TwT
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), 0);
	glBufferData(GL_ARRAY_BUFFER, sizeof(lines), lines, GL_DYNAMIC_DRAW);

	// glEnableVertexAttribArray(0);
	// set uniforms
	// unsigned int uniLoc ;
	unsigned int uniLoc = glGetUniformLocation(solidColour->id, "model");
	glUniformMatrix4fv(uniLoc, 1, GL_FALSE, glm::value_ptr(glm::mat4(1)));
	uniLoc = glGetUniformLocation(solidColour->id, "view");
	glUniformMatrix4fv(uniLoc, 1, GL_FALSE, glm::value_ptr(view));
	uniLoc = glGetUniformLocation(solidColour->id, "projection");
	glUniformMatrix4fv(uniLoc, 1, GL_FALSE, glm::value_ptr(proj));
	// uniLoc = glGetUniformLocation(solidColour->id, "colour");
	// glUniformMatrix4fv(uniLoc, 1, GL_FALSE,
	// 				   glm::value_ptr(glm::vec4(1, 0, 1, 1)));

	// draw the things
	// glDrawArrays(GL_TRIANGLES, 0,6);
	glDrawArrays(GL_LINES, 0, sizeof(lines)/sizeof(glm::vec3));
	// glDisableVertexAttribArray(0);
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
