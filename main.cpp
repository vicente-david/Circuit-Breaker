#include <iostream>
#include "glad/gl.h"
#include "GLFW/glfw3.h"
#include "PxPhysicsAPI.h"
#include "RenderingSystem.h"
#include "PhysicsSystem.h"
#include "InputSystem.h"

class TestInput1 : public CallbackInterface {

	void keyCallback(int key, int scancode, int action, int mods) {
		if (key == GLFW_KEY_W && action == GLFW_PRESS) {
			std::cout << "W pressed" << std::endl;
		}
	}

	void mouseButtonCallback(int button, int action, int mods) {
		if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS) {
			std::cout << "left click" << std::endl;
		}
		else if (button == GLFW_MOUSE_BUTTON_2 && action == GLFW_RELEASE) {
			std::cout << "right click released" << std::endl;
		}
	}

	void cursorPositionCallback(double xpos, double ypos) {
		if (xpos > 50) {
			//std::cout << "x > 50" << std::endl;
		}
	}

	void scrollCallback(double xoffset, double yoffset) {
		if (yoffset < 0) {
			std::cout << "scroll down" << std::endl;
		}
	}

	void windowSizeCallback(int width, int height) {
		glViewport(0, 0, width, height);
	}

};


int main()
{
	auto renderer = std::make_unique<RenderingSystem>();
	renderer->initializeRenderer();
	


	PhysicsSystem physicsSys;

	InputSystem inputSystem;
	inputSystem.attachWindow(renderer->window);
	TestInput1 t1;
	inputSystem.setCallback(std::make_shared<TestInput1>(t1));

	// --Placeholder code--
	std::vector<Entity> entityList;
	entityList.reserve(465);

	for (int i = 0; i < 465; i++)
	{
		entityList.emplace_back();
		entityList.back().name = "untitled_entity";
		entityList.back().transform = physicsSys.transformList[i];
		entityList.back().model = NULL;
	}


	// time
	double t = 0.0;
	const double dt = 1.0 / 60.0; // simulate at 60fps
	double currentTime = glfwGetTime();
	double accumulator = 0.0;


	// Triangle vectors (positions + colors)
	float vert_data[] = {
		-1.0f, -1.0f, 0.0f,
		 1.0f, 0.5f, 0.5f, 
		 1.0f, -1.0f, 0.0f,
		 0.5f, 1.f, 0.5f,
		 0.0f,  1.0f, 0.0f,
		 0.5f, 0.5f, 1.f
	};

	// Bind and set VBO data
	renderer->initializeShaders(vert_data, sizeof(vert_data));
	renderer->initializeText();

	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	int framesPassed = 0;
	std::string fps = std::to_string(0);

	while (!glfwWindowShouldClose(renderer->window)) {

		// time
		double newTime = glfwGetTime();
		double frameTime = newTime - currentTime;
		currentTime = newTime;
		accumulator += frameTime;
		framesPassed++;

		

		// physics
		while (accumulator >= dt) {
			physicsSys.updatePhysics(dt);
			accumulator -= dt;
			t += dt;
		}

		if (t >= 1.0) {
			fps = std::to_string(static_cast<int>(std::round(framesPassed / t)));
			t -= 1.0;
			framesPassed = 0;
		}



		// rendering
		glfwPollEvents();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		renderer->shaderProg->use();
		glBindVertexArray(renderer->VBO);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		// render text
		renderer->textProg->use();
		RenderText(*renderer->textProg, renderer->textVAO, renderer->textVBO, "FPS: "+fps, 10.f, 1380.f, 1.0f, glm::vec3(1.0f), renderer->textFont);
		
		glfwSwapBuffers(renderer->window);

	
	}
	glfwTerminate();
	

	return 0;
}

