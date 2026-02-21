#include "Entity.h"
#include "GLFW/glfw3.h"
#include "GameState.h"
#include "graphics/CameraComp.h"
#include "graphics/CameraSystem.h"
#include "InputSystem.h"
#include "PxPhysicsAPI.h"
#include "PxRigidDynamic.h"
#include "audio/AudioEngine.h"
#include "audio/Sound.h"
#include "debugUtils/Logger.h"
#include "ecs/Component.h"
#include "ecs/EntityManager.h"
#include "glad/gl.h"
#include "graphics/Camera.h"
#include "graphics/Model.h"
#include "graphics/RenderingSystem.h"
#include "physics/PhysicsManager.h"
#include "physics/PhysicsSystem.h"
#include "vehicles/ControllerSys.h"
#include "vehicles/SparkComponents.h"
#include "vehicles/SparkSys.h"
#include "vehicles/Vehicle.h"
#include <AL/al.h>
#include <cstdio>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>


int main() {
	// change to enable logging of different levels (0-> everything, 1->
	// warnings, 3-> errors, -1-> things that get spamed every frame)
	dbug::minLogSeverity = -1;
	dbug::logIgnore("INPUT");
	// dbug::logIgnore("ECS");
	dbug::logIgnoreType = dbug::WHITE_LIST;

	dbug::loggerInit();

	GameState gameState;
	// create the coordinator
	// initialize coordinator
	gameState.coordinator->Init();

	// register components
	gameState.coordinator->registerComponent<Transform>();
	gameState.coordinator->registerComponent<physx::PxRigidBody *>();
	gameState.coordinator->registerComponent<Model>();
	gameState.coordinator->registerComponent<SparkControls>();
	gameState.coordinator->registerComponent<SparkData>();
	gameState.coordinator->registerComponent<HumanController>();
	gameState.coordinator->registerComponent<CameraComp>();

	// register systems
	auto physicsSystem = PhysicsSystem::registerSystem(gameState.coordinator);
	auto renderer = RenderingSystem::registerSystem(gameState.coordinator);
	auto sparkSys = SparkSys::registerSystem(gameState.coordinator);
	auto controllerSys = ControllerSys::registerSystem(gameState.coordinator);
	auto cameraSys = CameraSystem::registerSystem(gameState.coordinator);


	// create physics manager
	std::shared_ptr<PhysicsManager> physicsManager =
		std::make_shared<PhysicsManager>();
	gameState.physics = physicsManager;

	InputSystem inputSystem;
	inputSystem.attachWindow(renderer->window);

	Actions gameActions = inputSystem.getActions();

	// time
	double t = 0.0;
	const double dt = 1.0 / 60.0; // simulate at 60fps
	// if its slower than this, just slow down the game instead of lagging even
	// more
	const double minFps = 30.0;
	double currentTime = glfwGetTime();
	double accumulator = 0.0;

	renderer->initializeShaders(); // Create shader programs
	renderer->initializeText();

	// Create models
	Model cube("assets/cube.obj");
	Model spark("assets/spark.obj");
	Model trackModel("assets/plane.obj"); // temporary track model


	// create track as a static mesh with baked physics
	Transform none = {glm::vec3(0, 0, 0), glm::quat(0, 0, 0, 0)};

	EcsEntity track = gameState.coordinator->createEntity();
	gameState.coordinator->addComponent(track, none);
	gameState.coordinator->addComponent(track, trackModel);
	physicsManager->initStaticMesh(trackModel.GetMesh()[0], none);
	dbug::log(0, "track entity id:%d", track);

	physicsManager->createTestObjs(*gameState.coordinator);

	// place holder test sounds
	Sound testSound = gameState.audio->createSound("muteCity");
	testSound.setLooping(true);
	testSound.start();
	float soundX = 0;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);

	int framesPassed = 0;
	std::string fps = std::to_string(0);


	// create spark with new system
		auto sparkEntity = sparkSys->createSpark(gameState);
		gameState.coordinator->addComponent(sparkEntity, HumanController{0});
		gameState.coordinator->addComponent(sparkEntity, CameraComp());

		auto testSpark2 = sparkSys->createSpark(gameState);
	// RENDER LOOP
	dbug::log(0, "Starting game loop");
	while (!glfwWindowShouldClose(renderer->window)) {

		// time
		double newTime = glfwGetTime();
		double frameTime = newTime - currentTime;
		currentTime = newTime;
		accumulator += frameTime;
		accumulator = std::min(accumulator, 1 / minFps);
		framesPassed++;

		// input
		gameActions = inputSystem.getActions();
		gameState.inputActions = gameActions;
		controllerSys->update(gameState);

		// physics
		while (accumulator >= dt) {

			sparkSys->updateSparks(dt, gameState);
			physicsSystem->updatePhysics(dt, gameState);
			cameraSys->update(gameState, dt);
			accumulator -= dt;
			t += dt;

			// dopler shift test
			// this stuff would  go in whatever is playing a sound (ex. physics
			// collision and like gamestate stuff for

			// test moving the sound left/right
			float soundVel = 0;
			if (gameActions.shimmyLeft) {
				soundX -= 0.5f;
				soundVel = -15;
				gameActions.shimmyLeft = false;
			}
			if (gameActions.shimmyRight) {
				soundX += 0.5f;
				soundVel = 15;
				gameActions.shimmyRight = false;
			}
			gameState.audio->updateSoundLoc(testSound, soundX, 0, 0);
			gameState.audio->updateSoundVel(testSound, soundVel, 0, 0);


		}

		if (t >= 1.0) {
			fps =
				std::to_string(static_cast<int>(std::round(framesPassed / t)));
			t -= 1.0;
			framesPassed = 0;
		}

		// rendering
		
		renderer->update(gameState, fps, cameraSys);

		gameState.audio->update(dt);
	}
	gameState.audio->close();
	glfwTerminate();

	return 0;
}
