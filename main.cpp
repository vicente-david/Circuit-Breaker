#include <iostream>
#include "glad/gl.h"
#include "GLFW/glfw3.h"
#include "PxPhysicsAPI.h"
#include "RenderingSystem.h"
#include "PhysicsSystem.h"


int main()
{
	auto renderer = std::make_unique<RenderingSystem>();
	renderer->initializeRenderer();
	


	PhysicsSystem physicsSys;

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
		 0.5f, 0.5f, 0.5f, 
		 1.0f, -1.0f, 0.0f,
		 0.5f, 0.5f, 0.5f,
		 0.0f,  1.0f, 0.0f,
		 0.5f, 0.5f, 0.5f
	};

	// Bind and set VBO data
	renderer->initializeShaders(vert_data, sizeof(vert_data));
	renderer->initializeText();

	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	

	while (!glfwWindowShouldClose(renderer->window)) {

		// time
		double newTime = glfwGetTime();
		double frameTime = newTime - currentTime;
		currentTime = newTime;
		accumulator += frameTime;

		std::cout << t << std::endl;
		std::string fps = std::to_string(static_cast<int>(std::round(1.0 / accumulator)));

		// physics
		while (accumulator >= dt) {
			physicsSys.updatePhysics(dt);
			accumulator -= dt;
			t += dt;
		}


		// rendering
		glfwPollEvents();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		renderer->shaderProg->use();
		glBindVertexArray(renderer->VBO);
		// if we use different shaders we'll need a way to know which one to use
		glDrawArrays(GL_TRIANGLES, 0, 3);

		// render text
		renderer->textProg->use();
		RenderText(*renderer->textProg, renderer->textVAO, renderer->textVBO, "FPS: "+fps, 50.f, 800.f, 5.0f, glm::vec3(1.0f), renderer->textFont);
		
		glfwSwapBuffers(renderer->window);

	
	}
	glfwTerminate();
	

	return 0;
}

