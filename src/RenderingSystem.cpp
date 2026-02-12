#include "RenderingSystem.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


void framebuffer_size_callback(GLFWwindow* window, int width, int height); //TODO: move this

RenderingSystem::RenderingSystem() : textVBO(1), textVAO(1)
{
	// Instantiate GLFW window
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Create window object
	window = glfwCreateWindow(800, 600, "Circuit Breaker", NULL, NULL);
	if (window == NULL)
	{
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

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}



void RenderingSystem::initializeShaders() {
	// Create shader program
	basicShader = std::make_unique<ShaderProgram>("shaders/basic.vert", "shaders/basic.frag");
	textProg = std::make_unique<ShaderProgram>("shaders/testText.vert", "shaders/testText.frag");
	textFont = initFont("assets/miamanueva.ttf");
	textMat = glm::ortho(0.0f, static_cast<float>(1440), 0.0f, static_cast<float>(1440));
	textProg->use();
	glUniformMatrix4fv(glGetUniformLocation(textProg->id, "projection"), 1, GL_FALSE, glm::value_ptr(textMat));

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

void RenderingSystem::update(std::vector<Entity> entities, std::string fps, Camera& c1) {
	
	basicShader->use();

	glm::mat4 view = glm::mat4(1.0f);
	//view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
	view = c1.GetViewMatrix();
	glm::mat4 proj;
	proj = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	unsigned int viewLoc = glGetUniformLocation(basicShader->id, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	unsigned int projLoc = glGetUniformLocation(basicShader->id, "projection");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));


	for (auto& entity : entities) {
		Model model = *entity.model; // get model of entity
		Transform transform = *entity.transform; // get transform of entity

		glm::mat4 modelTransform = glm::mat4(1.0f);
		modelTransform = glm::translate(modelTransform, transform.pos);
		modelTransform *= glm::toMat4(transform.rot);

		// use transformations
		unsigned int modelLoc = glGetUniformLocation(basicShader->id, "model");
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelTransform));
		
		model.Draw();
		
	}
	// render text
	textProg->use();
	RenderText(textProg->id, textVAO, textVBO, "FPS: " + fps, 10.f, 1380.f, 1.0f, glm::vec3(1.0f), textFont);

	glfwPollEvents();
	glfwSwapBuffers(window);
}