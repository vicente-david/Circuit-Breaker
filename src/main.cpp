#include <iostream>
#include "glad/gl.h"
#include "GLFW/glfw3.h"
#include "PxPhysicsAPI.h"
#include "RenderingSystem.h"
#include "PhysicsSystem.h"
#include "InputSystem.h"
#include "GameState.h"
#include "Model.h"
#include <glm/gtc/type_ptr.hpp>
#include "Camera.h"
#include "Vehicle.h"

int main()
{
	auto renderer = std::make_unique<RenderingSystem>();	
	PhysicsSystem physicsSys;
	GameState gameState;

	Vehicle car1(physicsSys);
	car1.init();
	car1.changeEngineDriveParams("TestDrive.json");

	InputSystem inputSystem;
	inputSystem.attachWindow(renderer->window);

	Actions gameActions = inputSystem.getActions();
	Camera c1 = Camera();
	

	// time
	double t = 0.0;
	const double dt = 1.0 / 60.0; // simulate at 60fps
	// if its slower than this, just slow down the game instead of lagging even more
	const double minFps = 30.0; 
	double currentTime = glfwGetTime();
	double accumulator = 0.0;


	renderer->initializeShaders(); // Create shader programs

	renderer->initializeText();

	// Create cube object
	Model cube("assets/cube.obj");
	
	// --Placeholder code--
	std::vector<Entity> entityList;
	entityList.reserve(465);

	for (int i = 0; i < 465; i++)
	{
		entityList.emplace_back();
		entityList.back().name = "perro cube";
		entityList.back().transform = physicsSys.transformList[i];
		entityList.back().model = &cube;
	}


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
		accumulator = std::min(accumulator, 1/minFps);
		framesPassed++;


		// input

		gameActions = inputSystem.getActions();
		c1.updateCamera(gameActions, accumulator);

		// physics
		while (accumulator >= dt) {
			car1.step(dt);
			physicsSys.updatePhysics(dt, entityList);
			accumulator -= dt;
			t += dt;
		}

		if (t >= 1.0) {
			fps = std::to_string(static_cast<int>(std::round(framesPassed / t)));
			t -= 1.0;
			framesPassed = 0;
		}
		
		// rendering
		renderer->update(entityList, fps, c1);


	}
	car1.cleanup();
	glfwTerminate();
	

	return 0;
}

