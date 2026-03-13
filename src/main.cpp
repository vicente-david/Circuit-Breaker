#pragma once
#include "GameState.h"
#include "GLFW/glfw3.h"
#include "InputSystem.h"
#include "PxShape.h"
#include "ai/AIControllerSys.h"
#include "ai/AISparkComponents.h"
#include "audio/AudioEngine.h"
#include "audio/AudioSystem.h"
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

#include "world/RespawnSystem.h"
#include <AL/al.h>
#include <cstdio>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include "world/CurveLoader.h"
#include "world/LapSystem.h"
#include "world/Track.h"
#include "ui/UISystem.h"
#include "Game.h"

int main() {

	// change to enable logging of different levels (0-> everything, 1->
	// warnings, 3-> errors, -1-> things that get spamed every frame)
	dbug::minLogSeverity = 0;
	// dbug::logIgnore("INPUT");
	// dbug::logIgnore("GAME");
	dbug::logIgnore("AI");
	// dbug::logListType = dbug::WHITE_LIST;
	//  dbug::logIgnore("ECS");
	// dbug::logIgnoreType = dbug::WHITE_LIST;

	dbug::loggerInit();

	GameState gameState;
	
	Game game = Game();
	game.initializeGame();

	std::shared_ptr<PhysicsSystem> physicsSystem = game.physicsSys;
	std::shared_ptr<RenderingSystem> renderer = game.renderer;
	std::shared_ptr<SparkSys> sparkSys = game.sparkSys;
	std::shared_ptr<ControllerSys> controllerSys = game.controllerSys;
	std::shared_ptr<CameraSystem> cameraSys = game.cameraSys;
	std::shared_ptr<AudioSystem> audioSystem = game.audioSys;
	std::shared_ptr<AIControllerSys> aiControllerSys = game.aiControllerSys;
	std::shared_ptr<RespawnSystem> respawnSystem = game.respawnSys;
	std::shared_ptr<LapSystem> lapSys = game.lapSys;
	std::shared_ptr<UISystem> uiSys = game.uiSys;

	


	//Entity r1 = gameState.coordinator->createEntity();
	//gameState.coordinator->addComponent<RectUI>(r1, RectUI());
	//gameState.coordinator->addComponent<UIComponent>(r1, UIComponent());
	


	// initialize debug panel
	// create physics manager
	std::shared_ptr<PhysicsManager> physicsManager =
		std::make_shared<PhysicsManager>();
	game.physics = physicsManager;

	gameState.physics = game.physics;
	gameState.coordinator = game.coordinator;
	gameState.audio = game.audio;
	gameState.uiSystem = game.uiSystem;

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
	//Model planeModel("assets/plane.obj");

	lapSys->generateCheckpoints("assets/biggertrack1.obj");

	// create the track. this should eventually be moved to its own
	// class/function
	Track Track("assets/biggertrack1.obj"); // loads model and paths
	// Track Track("assets/biggertrack1.obj"); // loads model and paths

	// Find max/min xyz coords of track for size of shadow map texture.
	//Mesh plMesh = planeModel.GetMesh()[0]; // only one mesh in track model
	
	renderer->setTrackBounds(Track.model.GetMesh()[0].GetBounds());

	// create track as a static mesh with baked physics
	{
		Transform none = {glm::vec3(0, 0, 0), glm::quat(0, 0, 0, 0)};
		Entity track = game.coordinator->createEntity();
		CollisionData trackPhys{GROUND, track};
		game.coordinator->addComponent(track, none);
		game.coordinator->addComponent(track, Track.model);
		game.coordinator->addComponent(track, trackPhys);

		Model wallsModel("assets/walls.obj"); // loads model and paths
		Entity walls = game.coordinator->createEntity();
		CollisionData planePhys{GROUND, walls};
		game.coordinator->addComponent(walls, none);
		game.coordinator->addComponent(walls, wallsModel);
		game.coordinator->addComponent(track, planePhys);

		auto trackActor =
			physicsManager->initStaticMesh(Track.model.GetMesh()[0], none);
		trackActor->userData = &trackPhys;

		for(auto& i : wallsModel.GetMesh()){
			auto actor = physicsManager->initStaticMesh(i, none);
			actor->userData = &planePhys;
		}

		dbug::log(0, "track entity id:%d", track);
	}


	// create finish line trigger box
	CollisionData finishCollisionData{FINISH_LINE, -1};
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
		triggerRect->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, false);
		triggerRect->setFlag(PxShapeFlag::eVISUALIZATION, true);

		triggerActor->attachShape(*triggerRect);
		triggerActor->userData = &finishCollisionData;
		physicsManager->gScene->addActor(*triggerActor);

		// give flag for finish, and it only collides with chassis 
		PxFilterData finishLineFilter(COLLISION_FLAG_FINISH,
									  COLLISION_FLAG_CHASSIS, 0, 0);
		triggerRect->setSimulationFilterData(finishLineFilter);
		// Clean up
		triggerRect->release();
	}

	// place holder test sounds
	Sound testSound = game.audio->createSound("muteCity");
	// alSourcef(testSound.source, AL_GAIN, 0.6f);
	testSound.setLooping(true);
	testSound.start();
	float soundX = 0;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);

	int framesPassed = 0;
	std::string fps = std::to_string(0);

	std::vector<TrackCurve> trackPaths = Track.paths; // set of paths
	glm::vec3 pathStartPt = trackPaths.at(0).curvePoints.at(0); // First point of first path (only one path for now)

	// create spark with new system
	PxVec3 startLoc = PxVec3(pathStartPt.x, pathStartPt.y + 2.f, pathStartPt.z - 6.f);
	auto sparkEntity = sparkSys->createSpark(gameState, startLoc);
	game.coordinator->addComponent(sparkEntity, HumanController{0});
	game.coordinator->addComponent(sparkEntity, CameraComp());
	game.coordinator->addComponent(sparkEntity, LapCounter());
	game.coordinator->addComponent(sparkEntity, Respawnable());
	game.coordinator->getComponent<SparkData>(sparkEntity).isHuman = !(0==1);
	game.coordinator->getComponent<LapCounter>(sparkEntity).isPlayer = true;

	gameState.uiText = game.uiSystem->raceUI(game.coordinator->getComponent<LapCounter>(sparkEntity).currentLap);


	startLoc = PxVec3(pathStartPt.x - 6.f, pathStartPt.y + 2.f, pathStartPt.z);
	auto testSpark2 = sparkSys->createSpark(gameState, startLoc);
	game.coordinator->addComponent(testSpark2, LapCounter());
	game.coordinator->addComponent(testSpark2, Respawnable());
	game.coordinator->addComponent(testSpark2, AIController{
		AIState::IDLE, // start AI in idle state
		trackPaths.at(0).curvePoints, // planned route
		trackPaths.at(0).curvatures, // angles at each point in route
		});

	startLoc = PxVec3(pathStartPt.x + 4.f, pathStartPt.y + 2.f, pathStartPt.z - 3.f);
	auto testSpark3 = sparkSys->createSpark(gameState, startLoc);
	game.coordinator->addComponent(testSpark3, LapCounter());
	game.coordinator->addComponent(testSpark3, Respawnable());
	game.coordinator->addComponent(testSpark3, AIController{
		AIState::IDLE, // start AI in idle state
		trackPaths.at(0).curvePoints, // planned route
		trackPaths.at(0).curvatures, // angles at each point in route
		0.10f, // curveBrakeThresh
		22.0f, // maxTargetSpeed
		0.02f, // curveBoostThresh
		8, // steeringSharpness
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
			game.physics->callbacks->resetLists();
			physicsSystem->updatePhysics(dt, gameState);
			cameraSys->update(gameState, dt);
			audioSystem->updateSounds(gameState);
			accumulator -= dt;
			t += dt;

			// dopler shift test
			// this stuff would  go in whatever is playing a sound (ex. physics
			// collision and like gamestate stuff for

			// // test moving the sound left/right
			// float soundVel = 0;
			// if (gameActions.shimmyLeft) {
			// 	soundX -= 0.5f;
			// 	soundVel = -15;
			// 	gameActions.shimmyLeft = false;
			// }
			// if (gameActions.shimmyRight) {
			// 	soundX += 0.5f;
			// 	soundVel = 15;
			// 	gameActions.shimmyRight = false;
			// }
			// position at 0,0,0 for testing
			game.audio->updateSoundLoc(testSound, 0, 0, 0);
			game.audio->updateSoundVel(testSound, 0, 0, 0);
		}

		// AI
		aiControllerSys->update(gameState);
		// after physics update
		lapSys->update(gameState);
		respawnSystem->update(gameState);
		

		if (t >= 1.0) {
			fps =
				std::to_string(static_cast<int>(std::round(framesPassed / t)));
			t -= 1.0;
			framesPassed = 0;
		}

		// rendering

		renderer->update(gameState, fps, cameraSys);

		game.audio->update(dt);
	}
	game.audio->close();
	dbugPanel::cleanup();
	glfwTerminate();

	return 0;
}
