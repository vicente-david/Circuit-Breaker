#include <AL/al.h>
#include <cstdio>
#include <iostream>
#include "audio/AudioEngine.h"
#include "glad/gl.h"
#include "GLFW/glfw3.h"
#include "PxPhysicsAPI.h"
#include "RenderingSystem.h"
#include "PhysicsSystem.h"
#include "InputSystem.h"
#include "GameState.h"
#include "Model.h"
#include "Texture.h"
#include <glm/gtc/type_ptr.hpp>
#include "Camera.h"

int main()
{
	auto renderer = std::make_unique<RenderingSystem>();	
	auto audio = std::make_unique<AudioEngine>();	
	PhysicsSystem physicsSys;
	GameState gameState;

	InputSystem inputSystem;
	inputSystem.attachWindow(renderer->window);

	Actions gameActions = inputSystem.getActions();
	Camera c1 = Camera();
	

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
	Model cube;
	cube.vertices = verts;
	cube.indices = indices;
	cube.textures.push_back({ texture, "diffuse" });
	cube.modelMatrix = glm::mat4(1.0f);

	// Temp transform data for cubes
	glm::vec3 cubePositions[] = {
	glm::vec3(0.0f,  0.0f,  0.0f),
	glm::vec3(2.0f,  5.0f, -15.0f),
	glm::vec3(-1.5f, -2.2f, -2.5f),
	glm::vec3(-3.8f, -2.0f, -12.3f),
	glm::vec3(2.4f, -0.4f, -3.5f),
	glm::vec3(-1.7f,  3.0f, -7.5f),
	glm::vec3(1.3f, -2.0f, -2.5f),
	glm::vec3(1.5f,  2.0f, -2.5f),
	glm::vec3(1.5f,  0.2f, -1.5f),
	glm::vec3(-1.3f,  1.0f, -1.5f)
	};

	// Temp function to create list of cube entities
	std::vector<Entity> objects{};
	for (int i = 0; i < 10; i++) {

		Transform* trans = new Transform();
		trans->pos = cubePositions[i];
		trans->rot = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

		Entity entity{ "perro" + std::to_string(i), PhysType::None, &cube, trans};
		objects.push_back(entity);

		// push back to entityList vector
		gameState.entityList.push_back(entity);

		std::cout << "Created entity " << i << " at position " << cubePositions[i].x << ", " << cubePositions[i].y << ", " << cubePositions[i].z << std::endl;
	}



	
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	
	int framesPassed = 0;
	std::string fps = std::to_string(0);


	Sound testSound = audio->createSound("hiya");
	testSound.setLooping(true);
	testSound.start();
	// RENDER LOOP
	while (!glfwWindowShouldClose(renderer->window)) {

		// time
		double newTime = glfwGetTime();
		double frameTime = newTime - currentTime;
		currentTime = newTime;
		accumulator += frameTime;
		framesPassed++;


		// input

		gameActions = inputSystem.getActions();
		c1.updateCamera(gameActions, accumulator);
		audio->updateListnerLoc(c1.Position.x, c1.Position.y, c1.Position.z);

		audio->updateSoundLoc(testSound, 0, 0, 0);

		// physics
		while (accumulator >= dt) {
			physicsSys.updatePhysics(dt);
			audio->update(dt);
			accumulator -= dt;
			t += dt;
		}


		if (t >= 1.0) {
			fps = std::to_string(static_cast<int>(std::round(framesPassed / t)));
			t -= 1.0;
			framesPassed = 0;
		}

		// test update object transforms
		objects[0].transform->rot = glm::quat(glm::vec3(0.7f, 0.5f, 0.1f) * (float)glfwGetTime());
		objects[1].transform->rot = glm::quat(glm::vec3(1.0f, 0.0f, 0.0f) * (float)glfwGetTime());
		objects[2].transform->rot = glm::quat(glm::vec3(2.0f, 1.0f, 0.0f) * (float)glfwGetTime());
		
		// rendering
		renderer->update(objects, VAO, fps, c1);


	}
	audio->close();
	glfwTerminate();
	

	return 0;
}

