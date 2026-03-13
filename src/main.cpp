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
	
	Game game = Game();
	GameState& gameState = game.gameState;
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

	game.initializeTrack();
	game.initializeFinishLine();

	

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

	//gameState.uiText = game.uiSystem->raceUI(game.coordinator->getComponent<LapCounter>(sparkEntity).currentLap);

	
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
