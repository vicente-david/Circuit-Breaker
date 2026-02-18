#pragma once

#include "Entity.h"

class GameState {
public:

	// Public functions
	GameState() {}
	Entity* addEntity(std::string name, PhysType physType, Model* model, Transform* transform);
	//void removeEntity(Entity* e);  <-- probably not necessary, might help optimize the game if we simply stop rendering certain entities instead.
	Entity* findEntity(std::string name);
	void endGame(Entity* gameWinner);
	void resetGameState();


	// Entity tracking
	std::vector<Entity> entityList;
	Entity* winner;

	// Flags
	bool gameEnded = false;

	// Game Parameters
	int numPlayers = 1;
	int numSparks = 8;
	int numActivePlayers = 8;
	int numLaps = 3;
};
