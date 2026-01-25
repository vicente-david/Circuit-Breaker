#include <iostream>
#include "glad/gl.h"
#include "GLFW/glfw3.h"
#include "PxPhysicsAPI.h"
#include "RenderingSystem.h"
#include "PhysicsSystem.h"


int main()
{
	auto renderer = std::make_unique<RenderingSystem>();

	glm::mat4(1.0f);
	renderer->initializeRenderer();
	


	PhysicsSystem physicsSys;


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
		glfwPollEvents();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		renderer->shaderProg->use();
		glBindVertexArray(renderer->VBO);
		// if we use different shaders we'll need a way to know which one to use

		glDrawArrays(GL_TRIANGLES, 0, 3);

		// render text
		renderer->textProg->use();
		
		RenderText(*renderer->textProg, renderer->textVAO, renderer->textVBO, "hi friends", 50.f, 800.f, 5.0f, glm::vec3(1.0f), renderer->textFont);
		

		glfwSwapBuffers(renderer->window);
	
	}
	glfwTerminate();
	
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
	// --------------------
	// Simulate at 60fps
	while (1)
	{
		physicsSys.gScene->simulate(1.0f / 60.0f);
		physicsSys.gScene->fetchResults(true);
		physicsSys.updateTransforms();
	}

	return 0;
}

