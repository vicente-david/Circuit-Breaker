#include "Game.h"

Game::Game() {
	coordinator = std::make_shared<Coordinator>();
	audio = std::make_shared<AudioEngine>();
	gameState = GameState();
}

// in theory this should initialize all internal systems
// should not do any track loading or spawning of entities
void Game::initializeGame() {
	initializeECS(); //initialize ecs

	// initialize debug panel
	// create physics manager
	std::shared_ptr<PhysicsManager> physicsManager =
		std::make_shared<PhysicsManager>();
	physics = physicsManager;

	gameState.physics = physics;
	gameState.coordinator = coordinator;
	gameState.audio = audio;
	gameState.uiSystem = uiSystem;

	initializeAudio();

	renderer->initializeShaders(); // Create shader programs
	renderer->initializeText();
	

	inputSystem.attachWindow(renderer->window);

	gameActions = inputSystem.getActions();

}

void Game::initializeECS() {
	// create the coordinator
	// initialize coordinator
	coordinator->Init();

	// register components
	coordinator->registerComponent<Transform>();
	coordinator->registerComponent<physx::PxRigidBody*>();
	coordinator->registerComponent<Model>();
	coordinator->registerComponent<SparkControls>();
	coordinator->registerComponent<SparkData>();
	coordinator->registerComponent<HumanController>();
	coordinator->registerComponent<CameraComp>();
	coordinator->registerComponent<LapCounter>();
	coordinator->registerComponent<AIController>();
	coordinator->registerComponent<CollisionData>();
	coordinator->registerComponent<Sound>();
	coordinator->registerComponent<Respawnable>();
	coordinator->registerComponent<UIComponent>();
	coordinator->registerComponent<RectUI>();

	// register systems
	physicsSys = PhysicsSystem::registerSystem(coordinator);
	renderer = RenderingSystem::registerSystem(coordinator);
	sparkSys = SparkSys::registerSystem(coordinator);
	controllerSys = ControllerSys::registerSystem(coordinator);
	cameraSys = CameraSystem::registerSystem(coordinator);
	audioSys = AudioSystem::registerSystem(coordinator);
	aiControllerSys = AIControllerSys::registerSystem(coordinator);
	respawnSys = RespawnSystem::registerSystem(coordinator);
	lapSys = LapSystem::registerSystem(coordinator);
	uiSys = UISystem::registerSystem(coordinator);

}

void Game::initializeRace() {
	initializeTrack();
	initializeFinishLine();
	renderer->renderPasses.push_back(&RenderingSystem::renderShadows); // start rendering it lol
}

void Game::initializeTrack() {
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
		Transform none = { glm::vec3(0, 0, 0), glm::quat(0, 0, 0, 0) };
		Entity track = coordinator->createEntity();
		CollisionData trackPhys{ GROUND, track };
		coordinator->addComponent(track, none);
		coordinator->addComponent(track, Track.model);
		coordinator->addComponent(track, trackPhys);

		Model wallsModel("assets/walls.obj"); // loads model and paths
		Entity walls = coordinator->createEntity();
		CollisionData planePhys{ GROUND, walls };
		coordinator->addComponent(walls, none);
		coordinator->addComponent(walls, wallsModel);
		coordinator->addComponent(track, planePhys);

		auto trackActor =
			physics->initStaticMesh(Track.model.GetMesh()[0], none);
		trackActor->userData = &trackPhys;

		for (auto& i : wallsModel.GetMesh()) {
			auto actor = physics->initStaticMesh(i, none);
			actor->userData = &planePhys;
		}

		dbug::log(0, "track entity id:%d", track);
	}


	std::vector<TrackCurve> trackPaths = Track.paths; // set of paths
	glm::vec3 pathStartPt = trackPaths.at(0).curvePoints.at(0); // First point of first path (only one path for now)

	// initialize players
	initializePlayerSpark(trackPaths, pathStartPt + glm::vec3(0.0f, 2.0f, -6.0f));
	initializeAISpark(trackPaths, pathStartPt + glm::vec3(-6.0f, 2.0f, 0.0f));
	initializeAISpark2(trackPaths, pathStartPt + glm::vec3(4.0f, 2.0f, -3.0f));


	gameState.uiText = gameState.uiSystem->raceUI(coordinator->getComponent<LapCounter>(player).currentLap);

}

void Game::initializePlayerSpark(std::vector<TrackCurve>& trackPaths, glm::vec3& pathStartPt) {
	// create spark with new system
	PxVec3 startLoc = PxVec3(pathStartPt.x, pathStartPt.y, pathStartPt.z);
	Entity sparkEntity = sparkSys->createSpark(gameState, startLoc);
	coordinator->addComponent(sparkEntity, HumanController{ 0 });
	coordinator->addComponent(sparkEntity, CameraComp());
	coordinator->addComponent(sparkEntity, LapCounter());
	coordinator->addComponent(sparkEntity, Respawnable());
	coordinator->getComponent<SparkData>(sparkEntity).isHuman = !(0 == 1);
	coordinator->getComponent<LapCounter>(sparkEntity).isPlayer = true;
	player = sparkEntity;
}

void Game::initializeAISpark(std::vector<TrackCurve>& trackPaths, glm::vec3& pathStartPt) {
	PxVec3 startLoc = PxVec3(pathStartPt.x, pathStartPt.y, pathStartPt.z);
	Entity testSpark2 = sparkSys->createSpark(gameState, startLoc);
	coordinator->addComponent(testSpark2, LapCounter());
	coordinator->addComponent(testSpark2, Respawnable());
	coordinator->addComponent(testSpark2, AIController{
		AIState::IDLE, // start AI in idle state
		trackPaths.at(0).curvePoints, // planned route
		trackPaths.at(0).curvatures, // angles at each point in route
		});
}

void Game::initializeAISpark2(std::vector<TrackCurve>& trackPaths, glm::vec3& pathStartPt) {
	PxVec3 startLoc = PxVec3(pathStartPt.x, pathStartPt.y, pathStartPt.z);
	auto testSpark3 = sparkSys->createSpark(gameState, startLoc);
	coordinator->addComponent(testSpark3, LapCounter());
	coordinator->addComponent(testSpark3, Respawnable());
	coordinator->addComponent(testSpark3, AIController{
		AIState::IDLE, // start AI in idle state
		trackPaths.at(0).curvePoints, // planned route
		trackPaths.at(0).curvatures, // angles at each point in route
		0.10f, // curveBrakeThresh
		22.0f, // maxTargetSpeed
		0.02f, // curveBoostThresh
		8, // steeringSharpness
		});
}

void Game::initializeFinishLine() {
	// create finish line trigger box
	CollisionData finishCollisionData{ FINISH_LINE, -1 };
	{
		PxVec3 finishLinePosition(0.0f, 0.0f,
			10.0f); // finish line position in world space
		PxVec3 triggerLengths(
			255.637f, 100.0f,
			1.0f); // width, height, and depth of the finish line
		PxRigidStatic* triggerActor =
			physics->gPhysics->createRigidStatic(
				PxTransform(finishLinePosition)); // create static rigid body
		// for the trigger box

		auto material = physics->gPhysics->createMaterial(0, 0, 0);
		physx::PxShape* triggerRect = physics->gPhysics->createShape(
			physx::PxBoxGeometry(triggerLengths), *material, true);

		// set the shape as a trigger
		triggerRect->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
		triggerRect->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);
		triggerRect->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, false);
		triggerRect->setFlag(PxShapeFlag::eVISUALIZATION, true);

		triggerActor->attachShape(*triggerRect);
		triggerActor->userData = &finishCollisionData;
		physics->gScene->addActor(*triggerActor);

		// give flag for finish, and it only collides with chassis 
		PxFilterData finishLineFilter(COLLISION_FLAG_FINISH,
			COLLISION_FLAG_CHASSIS, 0, 0);
		triggerRect->setSimulationFilterData(finishLineFilter);
		// Clean up
		triggerRect->release();
	}
}

void Game::initializeAudio() {
	// place holder test sounds
	testSound = gameState.audio->createSound("muteCity");
	// alSourcef(testSound.source, AL_GAIN, 0.6f);
	testSound.setLooping(true);
	testSound.start();
	//float soundX = 0;
}

// update stuff
void Game::update() {
	// update inputs
	// input
	gameActions = inputSystem.getActions();
	gameState.inputActions = gameActions;
	controllerSys->update(gameState);

	if (gameActions.intializeGame) {
		initializeRace();
		gameActions.intializeGame = false;
	}

	updateTime();
	updatePhysics();
	// AI
	aiControllerSys->update(gameState);
	// after physics update
	lapSys->update(gameState);
	respawnSys->update(gameState);

	updateFPS();

	updateRendering();

}

void Game::updateTime() {
	// time
	double newTime = glfwGetTime();
	double frameTime = newTime - currentTime;
	currentTime = newTime;
	accumulator += frameTime;
	accumulator = std::min(accumulator, 1 / minFps);
	framesPassed++;
}

void Game::updatePhysics() {

	// physics
	while (accumulator >= dt) {

		sparkSys->updateSparks(dt, gameState);
		gameState.physics->callbacks->resetLists();
		physicsSys->updatePhysics(dt, gameState);
		cameraSys->update(gameState, dt);
		audioSys->updateSounds(gameState);
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

}

void Game::updateFPS() {

	if (t >= 1.0) {
		fps =
			std::to_string(static_cast<int>(std::round(framesPassed / t)));
		t -= 1.0;
		framesPassed = 0;
	}
}

void Game::updateRendering() {
	// rendering
	renderer->update(gameState, fps, cameraSys);
}

void Game::updateAudio() {
	gameState.audio->update(dt);
}