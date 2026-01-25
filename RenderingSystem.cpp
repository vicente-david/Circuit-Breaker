#include "RenderingSystem.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


void framebuffer_size_callback(GLFWwindow* window, int width, int height); //TODO: move this

RenderingSystem::RenderingSystem() : VAO(0), VBO(0), textVBO(1), textVAO(1), window(nullptr), shaderProg(nullptr)
{
	// Instantiate GLFW window
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Create window object
	window = glfwCreateWindow(800, 600, "window!", NULL, NULL);
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


// note: we can put all the data into one vbo, and then assign different vao's to different shaders
// ex: lighting needs normals, but text doesn't, we have different shaders for them anyways
// so just bind a different vao for lighting shader and a different vao for text shader
void RenderingSystem::initializeRenderer() {
	// Initialize and bind VAO
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	// Generate VBO
	glGenBuffers(1, &VBO);

	// bind VBO
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// color
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3*sizeof(float)));
	glEnableVertexAttribArray(1);
	

	// texture (no texture as of right now so it is offset 6 instead of 8)
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	// maybe in the future normals?

	// note: we don't have any data as of right now, we can pass that in later

}

void RenderingSystem::initializeShaders(float* vertices, int size) {
	// Create shader program
	glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
	shaderProg = std::make_unique<ShaderProgram>(std::string(SHADER_DIR) + "/basic.vert", std::string(SHADER_DIR) + "/basic.frag");
	textProg = std::make_unique<ShaderProgram>(std::string(SHADER_DIR) + "/testText.vert", std::string(SHADER_DIR) + "/testText.frag");
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