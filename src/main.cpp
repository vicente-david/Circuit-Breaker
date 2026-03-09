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
	gameState.coordinator->registerComponent<LapCounter>();
	gameState.coordinator->registerComponent<AIController>();
	gameState.coordinator->registerComponent<CollisionData>();
	gameState.coordinator->registerComponent<Sound>();
	gameState.coordinator->registerComponent<Respawnable>();
	gameState.coordinator->registerComponent<UIComponent>();
	gameState.coordinator->registerComponent<RectUI>();

	// register systems
	auto physicsSystem = PhysicsSystem::registerSystem(gameState.coordinator);
	auto renderer = RenderingSystem::registerSystem(gameState.coordinator);
	auto sparkSys = SparkSys::registerSystem(gameState.coordinator);
	auto controllerSys = ControllerSys::registerSystem(gameState.coordinator);
	auto cameraSys = CameraSystem::registerSystem(gameState.coordinator);
	auto audioSystem = AudioSystem::registerSystem(gameState.coordinator);
	auto aiControllerSys = AIControllerSys::registerSystem(gameState.coordinator);
	auto respawnSystem = RespawnSystem::registerSystem(gameState.coordinator);
	auto lapSys = LapSystem::registerSystem(gameState.coordinator);
	auto uiSys = UISystem::registerSystem(gameState.coordinator);

	//Entity r1 = gameState.coordinator->createEntity();
	//gameState.coordinator->addComponent<RectUI>(r1, RectUI());
	//gameState.coordinator->addComponent<UIComponent>(r1, UIComponent());
	


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
		Entity track = gameState.coordinator->createEntity();
		CollisionData trackPhys{GROUND, track};
		gameState.coordinator->addComponent(track, none);
		gameState.coordinator->addComponent(track, Track.model);
		gameState.coordinator->addComponent(track, trackPhys);

		Model wallsModel("assets/walls.obj"); // loads model and paths
		Entity walls = gameState.coordinator->createEntity();
		CollisionData planePhys{GROUND, walls};
		gameState.coordinator->addComponent(walls, none);
		gameState.coordinator->addComponent(walls, wallsModel);
		gameState.coordinator->addComponent(track, planePhys);

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
	Sound testSound = gameState.audio->createSound("muteCity");
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
	gameState.coordinator->addComponent(sparkEntity, HumanController{0});
	gameState.coordinator->addComponent(sparkEntity, CameraComp());
	gameState.coordinator->addComponent(sparkEntity, LapCounter());
	gameState.coordinator->addComponent(sparkEntity, Respawnable());
	gameState.coordinator->getComponent<SparkData>(sparkEntity).isHuman = !(0==1);

	gameState.uiText = gameState.uiSystem->raceUI(gameState.coordinator->getComponent<LapCounter>(sparkEntity).currentLap);


	startLoc = PxVec3(pathStartPt.x - 6.f, pathStartPt.y + 2.f, pathStartPt.z);
	auto testSpark2 = sparkSys->createSpark(gameState, startLoc);
	gameState.coordinator->addComponent(testSpark2, LapCounter());
	gameState.coordinator->addComponent(testSpark2, Respawnable());
	gameState.coordinator->addComponent(testSpark2, AIController{
		AIState::IDLE, // start AI in idle state
		trackPaths.at(0).curvePoints, // planned route
		trackPaths.at(0).curvatures, // angles at each point in route
		});

	startLoc = PxVec3(pathStartPt.x + 4.f, pathStartPt.y + 2.f, pathStartPt.z - 3.f);
	auto testSpark3 = sparkSys->createSpark(gameState, startLoc);
	gameState.coordinator->addComponent(testSpark3, LapCounter());
	gameState.coordinator->addComponent(testSpark3, Respawnable());
	gameState.coordinator->addComponent(testSpark3, AIController{
		AIState::IDLE, // start AI in idle state
		trackPaths.at(0).curvePoints, // planned route
		trackPaths.at(0).curvatures, // angles at each point in route
		0.10f, // curveBrakeThresh
		22.0f, // maxTargetSpeed
		0.02f, // curveBoostThresh
		8, // steeringSharpness
		});

	// extra spark to bully around
	 sparkSys->createSpark(gameState, PxVec3(-10,0,-40));
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
			gameState.physics->callbacks->resetLists();
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
			gameState.audio->updateSoundLoc(testSound, 0, 0, 0);
			gameState.audio->updateSoundVel(testSound, 0, 0, 0);
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
		gameState.uiText = gameState.uiSystem->raceUI(gameState.coordinator->getComponent<LapCounter>(sparkEntity).currentLap);

		renderer->update(gameState, fps, cameraSys);

		gameState.audio->update(dt);
	}
	gameState.audio->close();
	dbugPanel::cleanup();
	glfwTerminate();

	return 0;
}
