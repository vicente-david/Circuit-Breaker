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

	game.initializeGame();

	//Entity r1 = gameState.coordinator->createEntity();
	//gameState.coordinator->addComponent<RectUI>(r1, RectUI());
	//gameState.coordinator->addComponent<UIComponent>(r1, UIComponent());

	InputSystem inputSystem;
	inputSystem.attachWindow(game.renderer->window);

	Actions gameActions = inputSystem.getActions();
	// add debug imgui panel (needs to be after input callbacks are set)
	dbugPanel::createPanel(game.renderer->window);

	game.renderer->initializeShaders(); // Create shader programs
	game.renderer->initializeText();

	// Create models
	Model cube("assets/cube.obj");
	Model spark("assets/spark.obj");
	//Model planeModel("assets/plane.obj");

	game.initializeTrack();
	game.initializeFinishLine();


	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);

	
	// RENDER LOOP
	dbug::log(0, "Starting game loop");
	while (!glfwWindowShouldClose(game.renderer->window)) {

		
		//dbugPanel::debug::updateTime = frameTime;

		// input
		gameActions = inputSystem.getActions();
		game.gameState.inputActions = gameActions;
		game.controllerSys->update(game.gameState);

		game.update();
		
	}
	game.audio->close();
	dbugPanel::cleanup();
	glfwTerminate();

	return 0;
}
