#include "RenderingSystem.h"
#include <iostream>


void framebuffer_size_callback(GLFWwindow* window, int width, int height); //TODO: move this

RenderingSystem::RenderingSystem() : VAO(0), VBO(0), window(nullptr), shaderProg(nullptr)
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

	// Create shader program
	shaderProg = std::make_unique<ShaderProgram>(std::string(SHADER_DIR) + "/basic.vert", std::string(SHADER_DIR) + "/basic.frag");

	// Generate VBO
	glGenBuffers(1, &VBO);
	
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}