#include "Game.h"
#include "AllSystem.h"
#include "PxActor.h"
#include "physics/CollisionData.h"
#include "debugUtils/Panel.h"
#include <cmath>

Game::Game() {
	coordinator = std::make_shared<Coordinator>();
	audio = std::make_shared<AudioEngine>();
	pHelper = std::make_shared<ParticleHelper>();
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

	initializeAudio();
	renderer->initializeShaders(); // Create shader programs
	renderer->initializeLines();
	

	inputSystem.attachWindow(renderer->window);

	initializeUI();
	initializeParticles();
	
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
	coordinator->registerComponent<SparkSounds>();
	coordinator->registerComponent<HumanController>();
	coordinator->registerComponent<CameraComp>();
	coordinator->registerComponent<LapCounter>();
	coordinator->registerComponent<AIController>();
	coordinator->registerComponent<CollisionData>();
	coordinator->registerComponent<Respawnable>();
	coordinator->registerComponent<UIElement>();
	coordinator->registerComponent<Leaderboard>();
	coordinator->registerComponent<Animatable>();

	// register systems
	physicsSys = PhysicsSystem::registerSystem(coordinator);
	renderer = RenderingSystem::registerSystem(coordinator);
	sparkSys = SparkSys::registerSystem(coordinator);
	sparkSoundSys = SparkSoundSys::registerSystem(coordinator);
	controllerSys = ControllerSys::registerSystem(coordinator);
	cameraSys = CameraSystem::registerSystem(coordinator);
	aiControllerSys = AIControllerSys::registerSystem(coordinator);
	respawnSys = RespawnSystem::registerSystem(coordinator);
	leaderboardSys = LeaderboardSystem::registerSystem(coordinator);
	lapSys = LapSystem::registerSystem(coordinator);
	uiSys = UISystem::registerSystem(coordinator);
	particleSys = ParticleSystem::registerSystem(coordinator);

}

void Game::initializeRace() {
	raceEntities.clear(); // clear all players to ensure we start from a clean slate
	initializeTrack();
	// initializeFinishLine();
	renderer->renderPasses.push_back(&RenderingSystem::renderShadows); // start rendering it lol
	raceCountdown.start();
	lastPrintedSecond = -1;
}

void Game::initializeTrack() {
	//lapSys->generateCheckpoints("assets/curve.obj");

	// create the track. this should eventually be moved to its own
	// class/function
	Track track("assets/mainTrack.obj"); // loads model and paths
	// Track Track("assets/biggertrack1.obj"); // loads model and paths

	// Find max/min xyz coords of track for size of shadow map texture.
	//Mesh plMesh = planeModel.GetMesh()[0]; // only one mesh in track model

	renderer->setTrackBounds(track.model.GetMesh("Track").GetBounds());

	// create track as a static mesh with baked physics
	{
		Transform none = {glm::vec3(0, 0, 0), glm::quat(0, 0, 0, 0)};
		Entity trackE = gameState.coordinator->createEntity();
		raceEntities.push_back(trackE);
		gameState.coordinator->addComponent(trackE, none);
		gameState.coordinator->addComponent(trackE, track.model);
		gameState.coordinator->addComponent(trackE, CollisionData{GROUND, trackE});
		auto& trackPhys = gameState.coordinator->getComponent<CollisionData>(trackE);
		auto trackActor =
			physics->initStaticMesh(track.model.GetMesh("Track"), none);
		trackActor->userData = &trackPhys;

		//// add walls
		//Model wallsModel("assets/walls.obj"); // loads model and paths
		//Entity walls = coordinator->createEntity();
		//coordinator->addComponent(walls, none);
		//coordinator->addComponent(walls, wallsModel);
		//coordinator->addComponent(track, CollisionData{GROUND, walls});
		//CollisionData& planePhys = gameState.coordinator->getComponent<CollisionData>(walls);


		//for (auto& i : wallsModel.GetMeshes()) {
		//	auto actor = physics->initStaticMesh(i, none);
		//	actor->userData = &planePhys;
		//}

		//dbug::log(0, "track entity id:%d", track);

		// Ribbons
		Model ribbonModel("assets/ribbons.obj"); // loads model and paths
		Entity ribbon = gameState.coordinator->createEntity();
		raceEntities.push_back(ribbon);
		gameState.coordinator->addComponent(ribbon, none);
		gameState.coordinator->addComponent(ribbon, ribbonModel);
		gameState.coordinator->addComponent(ribbon, CollisionData{ GROUND, ribbon });
		auto& ribbonPhys = gameState.coordinator->getComponent<CollisionData>(ribbon);

		for (auto& i : ribbonModel.GetMeshes()) {
			auto actor = physics->initStaticMesh(i, none);
			actor->userData = &ribbonPhys;
		}

		// Healzone Tracks
		Model healTrackModel("assets/healzoneTracks.obj"); // loads model and paths
		Entity healTrack = gameState.coordinator->createEntity();
		raceEntities.push_back(healTrack);
		gameState.coordinator->addComponent(healTrack, none);
		gameState.coordinator->addComponent(healTrack, healTrackModel);
		gameState.coordinator->addComponent(healTrack, CollisionData{GROUND, healTrack});
		auto& healTrackPhys = gameState.coordinator->getComponent<CollisionData>(healTrack);

		for (auto& i : healTrackModel.GetMeshes()) {
			auto actor = physics->initStaticMesh(i, none);
			actor->userData = &healTrackPhys;
		}

		// Healzones
		Model healModel("assets/healzones.obj"); // loads model and paths
		Entity heal = gameState.coordinator->createEntity();
		raceEntities.push_back(heal);
		gameState.coordinator->addComponent(heal, none);
		gameState.coordinator->addComponent(heal, healModel);
		gameState.coordinator->addComponent(heal, CollisionData{ HEAL, heal });
		auto& healPhys = gameState.coordinator->getComponent<CollisionData>(heal);

		for(auto& i : healModel.GetMeshes()){
			auto actor = physics->initHealZones(i, none);
			actor->userData = &healPhys;
		}

		// Walls
		Model wallModel("assets/walls.obj"); // loads model and paths
		Entity wall = gameState.coordinator->createEntity();
		gameState.coordinator->addComponent(wall, none);
		gameState.coordinator->addComponent(wall, wallModel);
		gameState.coordinator->addComponent(wall, CollisionData{ WALL, wall });
		auto& wallPhys = gameState.coordinator->getComponent<CollisionData>(wall);

		for(auto& i : wallModel.GetMeshes()){
			auto actor = physics->initStaticMesh(i, none);
			actor->userData = &wallPhys;
		}

		// Kill Plane
		Model killModel("assets/killGround.obj"); // loads model and paths
		Entity kill = gameState.coordinator->createEntity();
		gameState.coordinator->addComponent(kill, none);
		gameState.coordinator->addComponent(kill, killModel);
		gameState.coordinator->addComponent(kill, CollisionData{ KILL, kill });
		auto& killPhys = gameState.coordinator->getComponent<CollisionData>(kill);

		for(auto& i : killModel.GetMeshes()){
			auto actor = physics->initStaticMesh(i, none, false);
			actor->userData = &killPhys;

		}
	}

	Track curve("assets/curve.obj");
	curve.paths[0].id = pFAST;
	std::vector<TrackCurve> trackPaths{ (curve.paths[0]) }; // set of paths
	glm::vec3 pathStartPt = trackPaths.at(0).curvePoints.at(0); // First point of first path (only one path for now)
	Track healCurve("assets/healCurve.obj");
	healCurve.paths[0].id = pHEAL;
	trackPaths.push_back(healCurve.paths[0]); // add heal path to trackpaths
	if (curve.paths[0].curvePoints.size() != healCurve.paths[0].curvePoints.size()) {
		dbug::log("DEF", 2, "\n\n WARNING: track curves of different lengths! \n\n");
	}
	lapSys->generateCheckpoints(trackPaths);
	aiControllerSys->setStatePaths(trackPaths); // send track paths to the ai controller
	respawnSys->setPaths(trackPaths);

	// initialize players
	initializePlayerSpark(trackPaths, pathStartPt + glm::vec3(-4.0f, 1.0f, -18.0f));
	initializeAISpark(trackPaths, pathStartPt + glm::vec3(4.0f, 1.0f, -3.0f), "P2");
	initializeAISpark(trackPaths, pathStartPt + glm::vec3(1.0f, 1.0f, -6.0f), "P3");
	initializeAISpark(trackPaths, pathStartPt + glm::vec3(-1.0f, 1.0f, -9.0f), "P4");
	initializeAISpark(trackPaths, pathStartPt + glm::vec3(-4.0f, 1.0f, -12.0f), "P5");
	initializeAISpark(trackPaths, pathStartPt + glm::vec3(4.0f, 1.0f, -9.0f), "P6");
	initializeAISpark(trackPaths, pathStartPt + glm::vec3(1.0f, 1.0f, -12.0f), "P7");
	initializeAISpark(trackPaths, pathStartPt + glm::vec3(-1.0f, 1.0f, -15.0f), "P8");
	
	p = { pathStartPt + glm::vec3(-4.0f, 2.0f, -18.0f), glm::vec4(0.9f, 0.9f, 0.9f, 0.8f), 0.01f, 5.f, glm::vec3(0.0f) };


	// Start countdown
	
	//gameState.uiText = gameState.uiSystem->raceUI(coordinator->getComponent<LapCounter>(player).currentLap);

}

void Game::initializePlayerSpark(std::vector<TrackCurve>& trackPaths, glm::vec3 pathStartPt) {
	// create spark with new system
	PxVec3 startLoc = PxVec3(pathStartPt.x, pathStartPt.y, pathStartPt.z);
	Entity sparkEntity = sparkSys->createSpark(gameState, startLoc, "Player");
	raceEntities.push_back(sparkEntity);
	coordinator->addComponent(sparkEntity, HumanController{ 0 });
	coordinator->addComponent(sparkEntity, CameraComp());
	coordinator->addComponent(sparkEntity, LapCounter());
	coordinator->addComponent(sparkEntity, Respawnable());
	coordinator->addComponent(sparkEntity, Leaderboard());
	coordinator->getComponent<SparkData>(sparkEntity).isHuman = !(0 == 1);
	coordinator->getComponent<LapCounter>(sparkEntity).isPlayer = true;
	player = sparkEntity;
	gameState.player = &player;

	// link health and boost to the UI system
	auto& sparkData = coordinator->getComponent<SparkData>(player);
	uiSys->playerHealth = &sparkData.health;
	uiSys->maxPlayerHealth = &sparkData.maxHealth;
	uiSys->playerBoost = &sparkData.boost;
	uiSys->maxPlayerBoost = &sparkData.maxBoost;
}

void Game::initializeAISpark(std::vector<TrackCurve>& trackPaths, glm::vec3 pathStartPt, std::string name) {
	PxVec3 startLoc = PxVec3(pathStartPt.x, pathStartPt.y, pathStartPt.z);
	Entity testSpark2 = sparkSys->createSpark(gameState, startLoc, name);
	raceEntities.push_back(testSpark2);
	coordinator->addComponent(testSpark2, LapCounter());
	coordinator->addComponent(testSpark2, Respawnable());
	coordinator->addComponent(testSpark2, Leaderboard());
	//coordinator->addComponent(testSpark2, AIController{
	//	AIDriveState::IDLE, // start AI in idle state
	//	trackPaths.at(0).curvePoints, // planned route
	//	trackPaths.at(0).curvatures, // angles at each point in route
	//	});
}

void Game::initializeAISpark2(std::vector<TrackCurve>& trackPaths, glm::vec3 pathStartPt) {
	PxVec3 startLoc = PxVec3(pathStartPt.x, pathStartPt.y, pathStartPt.z);
	auto testSpark3 = sparkSys->createSpark(gameState, startLoc, "AI P3");
	raceEntities.push_back(testSpark3);
	coordinator->addComponent(testSpark3, LapCounter());
	coordinator->addComponent(testSpark3, Respawnable());
	coordinator->addComponent(testSpark3, Leaderboard());
	coordinator->addComponent(testSpark3, AIController{
		AIDriveState::IDLE, // start AI in idle state
		trackPaths.at(0).curvePoints, // planned route
		trackPaths.at(0).curvatures, // angles at each point in route

		});
}

void Game::initializeFinishLine() {
	// return;
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
	music = gameState.audio->createSound("muteCity",false);
	music->volume(1.6);
	music->setLooping(true);
	music->start();
	//float soundX = 0;
}

void Game::initializeUI() {
	uiSys->window = renderer->window;
	uiSys->SCR_WIDTH = &renderer->SCR_WIDTH;
	uiSys->SCR_HEIGHT = &renderer->SCR_HEIGHT;
	uiSys->initializeRenderingParams();
	uiSys->coordinator = coordinator;
	uiSys->fps = &fps;
	uiSys->playerBackwards = &gameState.playerBackwards;

	uiSys->screenInitialization();

	uiSys->addScreen("fpsCounter");
	uiSys->addScreen("mainMenu");
	uiSys->selectedEntities();

	// give gameState access to uiSystem for controller-driven UI navigation
	gameState.uiSystem = uiSys;
}

void Game::initializeParticles() {
	particleSys->init();
	particleSys->proj = &renderer->projection;

	pHelper->connectSys(particleSys); // connect the particle system to the helper (gives a shared pointer to the helper)
	sparkSys->pHelper = pHelper; // give a pointer to the helper to systems that want to communicate with the particle system
}

void Game::handleMenuControl() {
	UIActions& ui = gameState.uiActions;

	if (gameState.nextState == END) return;

	if (ui.intializeGame) {
		ui.intializeGame = false;
		ui.menuControl = 1; // transition to gameplay
		uiSys->resetSelection();
		gameState.nextState = GAMEPLAY;
	}

	if (ui.menuControl == -1) {
		gameState.nextState = MAINMENU;
	}

	else if (ui.menuControl == 2) {
		gameState.nextState = PAUSED;
	}

	else if (ui.menuControl == 1) {
		gameState.nextState = GAMEPLAY;
	}


}

void Game::cleanupGame() {
	// remove alll physx actors for the sparks
	for (Entity e : raceEntities) {
		if (coordinator->hasComponent<SparkData>(e)) {
			SparkData& sData = coordinator->getComponent<SparkData>(e);
			if (sData.rBody)
				physics->gScene->removeActor(*sData.rBody);
		}
		coordinator->destroyEntity(e);
	}
	raceEntities.clear();

	// remove all the static actors from physx scene
	PxU32 nbActors = physics->gScene->getNbActors(PxActorTypeFlag::eRIGID_STATIC);
	if (nbActors > 0) {
		std::vector<PxActor*> actors(nbActors);
		physics->gScene->getActors(PxActorTypeFlag::eRIGID_STATIC, actors.data(), nbActors);
		for (PxActor* actor : actors)
			physics->gScene->removeActor(*actor);
	}

	renderer->renderPasses.clear();
	dbugPanel::clearSparkData(); // WONT BE NEEDED WITH UI IMPLEMENTATION I ASSUME
	gameState.resetGameState();
}

void Game::stateTransition() {
	// if a transition happened
	if (!(gameState.currentState == gameState.nextState)) {
		// we can have a switch case statement here
		// for every possible state

		// we know this is the "old" state
		// we can do cleanup here if necessary
		// outgoing transition
		// exit
		switch (gameState.currentState) {

			// we're likely initializing here, so pop the main menu from UI
			// initialize everything 
		case (MAINMENU):
			uiSys->popScreen();
			initializeRace();
			uiSys->addScreen("lapCounter");
			uiSys->addScreen("placeCounter");
			uiSys->addScreen("backwardsDisplay");
			uiSys->addScreen("myHealthIsDeclining");
			uiSys->addScreen("boostMeOffABridge");
			uiSys->addScreen("countDown");
			break;

			// our likely next state is paused or game ended
			// we may want to pop heads up display
		case (GAMEPLAY):
			uiSys->clearAllScreens();
			break;

			// we're likely resuming the game, so pop all pause menus
			// add back our gameplay uis (order matters)
		case (PAUSED):
			uiSys->clearAllScreens();
			uiSys->addScreen("fpsCounter");
			uiSys->addScreen("lapCounter");
			uiSys->addScreen("placeCounter");
			uiSys->addScreen("backwardsDisplay");
			uiSys->addScreen("myHealthIsDeclining");
			uiSys->addScreen("boostMeOffABridge");
			renderer->renderPasses.push_back(&RenderingSystem::renderShadows);
			break;

			// we're likely restarting the game
		case (END):
			break;
		}

		// this is the new state, do things that you want to update
		// incoming transition
		// entry
		switch (gameState.nextState) {
			// HOW DID WE GET HERE, this shouldn't run
		case (MAINMENU):
			cleanupGame();
			uiSys->clearAllScreens();
			uiSys->addScreen("mainMenu");
			uiSys->resetSelection();
			break;

			// we are resuming gameplay
		case (GAMEPLAY):
			//uiSys->addScreen("racingHUD");
			
			break;
			// we will be in a pause menu
		case (PAUSED):
			uiSys->clearAllScreens();
			uiSys->addScreen("fpsCounter");
			uiSys->addScreen("pauseMenu"); // ensure this menu is pushed last, so that it goes on top, else UI input wont work
			renderer->renderPasses.clear();
			break;

			// someone has finished the race
		case (END):
			uiSys->clearAllScreens();
			uiSys->createStandingsScreen(coordinator->getComponent<Leaderboard>(player));
			uiSys->addScreen("standingsScreen");
			break;
		}

		gameState.currentState = gameState.nextState;
	}
}


// update stuff
void Game::update() {
	// update inputs
	// input
	gameActions = inputSystem.getActions();
	gameUIActions = inputSystem.getUIActions();

	gameState.inputActions = gameActions;
	gameState.uiActions = gameUIActions;

	// controllerSys handles both vehicle controls AND UI navigation
	// UI navigation modifies game.uiActions directly (confirm, goBack, menuControl, etc.)
	// skip vehicle controls during countdown so the player can't move early
	if (!raceCountdown.activeTimer())
		controllerSys->update(gameState);

	// --- State transition from menuControl ---
	// Read back uiActions after controllerSys may have modified them
	UIActions& ui = gameState.uiActions;

	handleMenuControl();

	// sync menuControl back to InputSystem so controller/keyboard checks can see the current state
	inputSystem.setMenuControl(ui.menuControl);

	stateTransition(); // handle any state transitions

	updateTime();

	// do our updates only if in game
	if (gameState.currentState == GAMEPLAY) {

		// physics and camera always run so vehicles settle on the track
		updatePhysics();

		// race countdown (runs once per frame)
		if (raceCountdown.activeTimer()) {
			raceCountdown.update(frameTime);
			int secondsLeft = (int)std::ceil(raceCountdown.remaining); // round up to nearest second for displayed countdown
			if (secondsLeft <= 3.0 && secondsLeft != lastPrintedSecond) { // prevents output console spam
				lastPrintedSecond = secondsLeft;
				uiSys->updateCountdown(std::to_string(secondsLeft), frameTime);
				std::cout << "RACE START IN: " << secondsLeft << "\n";
			}
			if (raceCountdown.completedTimer()) {
				raceCountdown.resetTimer();
				uiSys->updateCountdown("GO!", frameTime);
				uiSys->goTimer.timerDuration = 2.0;
				uiSys->goTimer.start();
				std::cout << "GO!\n";
			}
		}
		else {
			if (uiSys->goTimer.activeTimer()) {
				uiSys->goTimer.update(frameTime);
			}
			else if (uiSys->go) {
				uiSys->popScreen();
				uiSys->go = false;
			}
			// countdown finished — race is live
			aiControllerSys->update(gameState);
			// after physics update
			lapSys->update(gameState);
			uiSys->updateLapCounter(coordinator->getComponent<LapCounter>(player).currentLap);
			respawnSys->update(gameState);
			leaderboardSys->update(gameState);
			uiSys->updatePlaceCounter(player);
			uiSys->updateBackwardsDisplay(frameTime);
		}
	}

	updateFPS();

	updateRendering();

}

void Game::updateTime() {
	// time
	double newTime = glfwGetTime();
	frameTime = newTime - currentTime;
	currentTime = newTime;
	accumulator += frameTime;
	accumulator = std::min(accumulator, 1 / minFps);
	framesPassed++;

	gameState.frameTime = &frameTime;
	t += frameTime;
}

void Game::updatePhysics() {

	// physics
	while (accumulator >= dt) {

		sparkSys->updateSparks(dt, gameState);
		gameState.physics->callbacks->resetLists();
		physicsSys->updatePhysics(dt, gameState);
		cameraSys->update(gameState, dt);
		sparkSoundSys->updateSounds(dt, gameState);
		audio->update(dt);
		accumulator -= dt;
		//t += dt;

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
		// gameState.audio->updateSoundLoc(testSound, 0, 0, 0);
		// gameState.audio->updateSoundVel(testSound, 0, 0, 0);
	}

}

void Game::updateFPS() {

	if (t >= 1.0) {
		fps =
			std::to_string(static_cast<int>(std::round(framesPassed / t)));
		t -= 1.0;
		framesPassed = 0;
	}

	uiSys->updateFPSCounter();
}

void Game::updateRendering() {
	// rendering
	renderer->update(gameState, fps, cameraSys);
	
	// update UI
	uiSys->update();
	particleSys->update(gameState, dt);

	glfwPollEvents();
	glfwSwapBuffers(renderer->window);
}
