#pragma once
#include "glad/gl.h"
#include "GLFW/glfw3.h"
#include "debugUtils/Logger.h"
#include "debugUtils/Panel.h"
#include <cstdio>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include "Game.h"

int main() {

	// change to enable logging of different levels (0-> everything, 1->
	// warnings, 3-> errors, -1-> things that get spamed every frame)
	dbug::minLogSeverity = 0;
	dbug::logIgnore("INPUT");
	dbug::logIgnore("GAME");
	//dbug::logIgnore("AI");
	//dbug::logIgnore("LEADERBOARD");
	// dbug::logListType = dbug::WHITE_LIST;
	dbug::logIgnore("ECS");
	dbug::logIgnore("GEN");
	dbug::logIgnore("AUDIO");
	dbug::logIgnore("REND");
	dbug::logIgnore("LAP");
	dbug::loggerInit();
	
	Game game = Game();

	game.initializeGame();

	// add debug imgui panel (needs to be after input callbacks are set)
	dbugPanel::createPanel(game.renderer->window);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);

	
	// RENDER LOOP
	dbug::log(0, "Starting game loop");
	while (!glfwWindowShouldClose(game.renderer->window)) {
		//dbugPanel::debug::updateTime = frameTime;
		game.update();
		
	}
	game.audio->close();
	dbugPanel::cleanup();
	glfwTerminate();

	return 0;
}
