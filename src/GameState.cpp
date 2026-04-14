#include "GameState.h"
#include "algorithm"
#include "ecs/EntityManager.h"
#include <cstdio>
#include <memory>
#include "vehicles/SparkComponents.h"

GameState::GameState() {
	// what's up i'm a useful function
}


void GameState::endGame(Entity gameWinner) {
	// Record the first finisher as the race winner
	if (!gameEnded) {
		winner = gameWinner;
		gameEnded = true;
		std::cout << "winner was found " << std::endl;
	}

	// Transition to END screen when the human player finishes
	if (coordinator->getComponent<SparkData>(gameWinner).isHuman){
		nextState = END;
		// finish sound effect
		auto sound = audio->createSound("finish");
		sound->volume(2.0);
		sound->start();
	}
}


void GameState::resetGameState() {
	winner = -1;			// reset winner
	gameEnded = false;			// reset game ended to false
	finishOrder.clear();		// clear finish order for next race
}
