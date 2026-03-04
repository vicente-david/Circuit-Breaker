#include "GameState.h"
#include "GLFW/glfw3.h"
#include "InputSystem.h"
#include "audio/AudioEngine.h"
#include "audio/Sound.h"
#include "debugUtils/Logger.h"
#include "debugUtils/Panel.h"
#include "ecs/Component.h"
#include "ecs/EntityManager.h"
#include "glad/gl.h"
#include "graphics/CameraComp.h"
#include "graphics/CameraSystem.h"
#include "graphics/Model.h"
#include "graphics/RenderingSystem.h"
#include "physics/PhysicsManager.h"
#include "physics/PhysicsSystem.h"
#include "vehicles/ControllerSys.h"
#include "vehicles/SparkComponents.h"
#include "vehicles/SparkSys.h"
#include "ai/AISparkComponents.h"
#include "ai/AIControllerSys.h"
#include <AL/al.h>
#include <cstdio>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include "world/Track.h"

int main() {

	// change to enable logging of different levels (0-> everything, 1->
	// warnings, 3-> errors, -1-> things that get spamed every frame)
	dbug::minLogSeverity = 0;
	// dbug::logIgnore("INPUT");
	dbug::logIgnore("GAME");
	//dbug::logIgnore("AI");
	//dbug::logListType = dbug::WHITE_LIST;
	// dbug::logIgnore("ECS");
	//dbug::logIgnoreType = dbug::WHITE_LIST;

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
	gameState.coordinator->registerComponent<AIController>();

	// register systems
	auto physicsSystem = PhysicsSystem::registerSystem(gameState.coordinator);
	auto renderer = RenderingSystem::registerSystem(gameState.coordinator);
	auto sparkSys = SparkSys::registerSystem(gameState.coordinator);
	auto controllerSys = ControllerSys::registerSystem(gameState.coordinator);
	auto cameraSys = CameraSystem::registerSystem(gameState.coordinator);
	auto aiControllerSys = AIControllerSys::registerSystem(gameState.coordinator);

	// initialize debug panel
	// create physics manager
	std::shared_ptr<PhysicsManager> physicsManager =
		std::make_shared<PhysicsManager>();
	gameState.physics = physicsManager;

	InputSystem inputSystem;
	inputSystem.attachWindow(renderer->window);

	Actions gameActions = inputSystem.getActions();
	// add debug imgui panel (needs to be after input callbacks are set)
	dbugPanel::createPanel(renderer->window);

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
	Model planeModel("assets/plane.obj");

	// create the track. this should eventually be moved to its own
	// class/function
	Track Track("assets/track1.obj"); //loads model and paths

	// Find max/min xyz coords of track for size of shadow map texture.
	Mesh plMesh = planeModel.GetMesh()[0]; // only one mesh in track model
	renderer->setTrackBounds(plMesh.GetBounds());

	// create track as a static mesh with baked physics
	Transform none = {glm::vec3(0, 0, 0), glm::quat(0, 0, 0, 0)};

	Entity track = gameState.coordinator->createEntity();
	gameState.coordinator->addComponent(track, none);
	gameState.coordinator->addComponent(track, Track.model);
	Entity plane = gameState.coordinator->createEntity();
	gameState.coordinator->addComponent(plane, none);
	gameState.coordinator->addComponent(plane, planeModel);
	physicsManager->initStaticMesh(Track.model.GetMesh()[0], none);
	physicsManager->initStaticMesh(planeModel.GetMesh()[0], none);
	dbug::log(0, "track entity id:%d", track);

	// create test object pyramid
	physicsManager->createTestObjs(*gameState.coordinator);

	// create finish line trigger box
	{
		PxVec3 finishLinePosition(0.0f, 0.0f,
								  10.0f); // finish line position in world space
		PxVec3 triggerLengths(
			255.637f, 100.0f,
			1.0f); // width, height, and depth of the finish line
		PxRigidStatic *triggerActor =
			physicsManager->gPhysics->createRigidStatic(
				PxTransform(finishLinePosition)); // create static rigid body
												  // for the trigger box

		auto material = physicsManager->gPhysics->createMaterial(0, 0, 0);
		physx::PxShape *triggerRect = physicsManager->gPhysics->createShape(
			physx::PxBoxGeometry(triggerLengths), *material, true);

		// set the shape as a trigger
		triggerRect->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
		triggerRect->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);

		triggerActor->attachShape(*triggerRect);
		// physicsManager->gScene->addActor(*triggerActor);

		PxFilterData finishLineTriggerFilterData;
		finishLineTriggerFilterData.word0 =
			99; // it detects only the player vehicle
		triggerRect->setSimulationFilterData(finishLineTriggerFilterData);
		// Clean up
		triggerRect->release();
	}

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
	PxVec3 startLoc = PxVec3(5.000000000f, -0.000000000f, -40.0f);
	auto sparkEntity = sparkSys->createSpark(gameState, startLoc);
	gameState.coordinator->addComponent(sparkEntity, HumanController{0});
	gameState.coordinator->addComponent(sparkEntity, CameraComp());

	PxVec3 startLoc2 = PxVec3(2.000000000f, -0.000000000f, -30.0f);
	auto testSpark2 = sparkSys->createSpark(gameState, startLoc2);
	gameState.coordinator->addComponent(testSpark2, AIController{
		AIState::IDLE, // start AI in idle state
		glm::vec3(2.0f, 0.0f, -20.0f), // target position
		1.0f, // arrival radius
		2.0f, // steering sharpness
		2.0f // brake distance
		});

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
		dbugPanel::debug::updateTime = frameTime;

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

		// AI
		aiControllerSys->update(gameState);

		// rendering

		renderer->update(gameState, fps, cameraSys);

		gameState.audio->update(dt);
	}
	gameState.audio->close();
	dbugPanel::cleanup();
	glfwTerminate();

	return 0;
}
