#include "GameState.h"
#include "algorithm"
#include "ecs/EntityManager.h"
#include <cstdio>
#include <memory>

GameState gameState;

GameState::GameState(){
		coordinator = std::make_shared<Coordinator>();
		audio = std::make_unique<AudioEngine>();
	}

void GameState::endGame(EcsEntity gameWinner) {
	winner = gameWinner;		// assign the game winner
								
								// TODO: probably do some UI/game management stuff here to give endscreen, etc.
								
	gameEnded = true;			// end the game
}

void GameState::resetGameState() {
	winner = -1;			// reset winner
	gameEnded = false;			// reset game ended to false

}
