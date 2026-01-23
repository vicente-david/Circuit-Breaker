#include <iostream>
#include "glad/gl.h"
#include "GLFW/glfw3.h"
#include "PxPhysicsAPI.h"
#include "RenderingSystem.h"
#include "PhysicsSystem.h"


int main()
{
	auto renderer = std::make_unique<RenderingSystem>();

	// Initialize and bind VAO
	glGenVertexArrays(1, &renderer->VAO);
	glBindVertexArray(renderer->VAO);


	PhysicsSystem physicsSys;


	// Triangle vectors
	static const GLfloat vert_data[] = {
		-1.0f, -1.0f, 0.0f,
		 1.0f, -1.0f, 0.0f,
		 0.0f,  1.0f, 0.0f
	};

	// Bind and set VBO data
	glBindBuffer(GL_ARRAY_BUFFER, renderer->VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vert_data), vert_data, GL_STATIC_DRAW);

	while (!glfwWindowShouldClose(renderer->window)) {
		glfwPollEvents();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		renderer->shaderProg->use();
		
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, renderer->VBO);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glDrawArrays(GL_TRIANGLES, 0, 3);
		glDisableVertexAttribArray(0);

		glfwSwapBuffers(renderer->window);
	
	}
	glfwTerminate();
	

	// Simulate at 60fps
	while (1)
	{
		physicsSys.gScene->simulate(1.0f / 60.0f);
		physicsSys.gScene->fetchResults(true);
	}

	return 0;
}

