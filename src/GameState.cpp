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
	winner = gameWinner;		// assign the game winner
								
								// TODO: probably do some UI/game management stuff here to give endscreen, etc.
	std::cout << "winner was found " << std::endl;

	if (coordinator->getComponent<SparkData>(gameWinner).isHuman) uiText = uiSystem->changeToWinScreen(); // todo fix lol
	else uiText = uiSystem->changeToLoseScreen();

	gameEnded = true;			// end the game
}


void GameState::resetGameState() {
	winner = -1;			// reset winner
	gameEnded = false;			// reset game ended to false

}
