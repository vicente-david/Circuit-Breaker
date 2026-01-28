#include <iostream>
#include "glad/gl.h"
#include "GLFW/glfw3.h"
#include "PxPhysicsAPI.h"
#include "RenderingSystem.h"
#include "PhysicsSystem.h"
#include "Model.h"
#include "Texture.h"
#include <glm/gtc/type_ptr.hpp>


int main()
{
	auto renderer = std::make_unique<RenderingSystem>();	
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


	// Cube test vertices (pos, col, tex)
	std::vector<Vertex> verts = {
		{glm::vec3(-0.5f,  -0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f)},
		{glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 0.0f)},
		{glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f)},
		{glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 1.0f)},

		{glm::vec3(-0.5f,  -0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f)},
		{glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 0.0f)},
		{glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f)},
		{glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 1.0f)},

		{glm::vec3(-0.5f,  -0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 0.0f)},
		{glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f)},

		{glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f)},
		{glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 1.0f)},

		{glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f)},
		{glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f)},

		{glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 1.0f)},
		{glm::vec3(-0.5f,  -0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 1.0f)},

	};

	std::vector<unsigned int> indices{
		0, 1, 2, 2, 3, 0,
		4, 5, 6, 6, 7, 4,
		0, 8, 9, 9, 3, 0,
		10, 1, 2, 2, 11, 10,
		12, 13, 2, 2, 3, 12,
		4, 5, 14, 14, 15, 4
	};


	renderer->initializeShaders(); // Create shader programs
	unsigned int VAO = renderer->initVAO(verts.data(), verts.size() * sizeof(Vertex), indices.data(), indices.size() * sizeof(unsigned int)); // Initialize VAO, VBO, EBO
	
	unsigned int texture = generateTexture("assets/textures/perro.jpg", true);
	
	renderer->initializeText();

	// Create cube object
	auto cube = std::make_unique<Model>();
	cube->vertices = verts;
	cube->indices = indices;
	cube->textures.push_back({ texture, "diffuse" });
	cube->modelMatrix = glm::mat4(1.0f);

	// Create list of objects, add cube
	std::vector<Model> objects;
	objects.push_back(*cube);
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	
	int framesPassed = 0;
	std::string fps = std::to_string(0);


	// RENDER LOOP
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

		// Cube transform
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(1.0f, 0.5f, 0.5f));
		objects[0].modelMatrix = model;

		// rendering
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
		renderer->update(objects, VAO);

		// render text
		renderer->textProg->use();
		RenderText(*renderer->textProg, renderer->textVAO, renderer->textVBO, "FPS: "+fps, 10.f, 1380.f, 1.0f, glm::vec3(1.0f), renderer->textFont);
		
		glfwPollEvents();
		glfwSwapBuffers(renderer->window);

	
	}
	glfwTerminate();
	

	return 0;
}

