#pragma once

#include "Entity.h"
#include "InputSystem.h"
#include "audio/AudioEngine.h"
#include "ecs/Coordinator.h"
#include "physics/PhysicsManager.h"

#include <memory>

class GameState {
  public:
	// general managers that things will often need to play sounds, add
	// entities, etc.
	std::shared_ptr<Coordinator> coordinator;
	std::shared_ptr<PhysicsManager> physics;
	std::unique_ptr<AudioEngine> audio;
	Actions inputActions;

	// Public functions
	GameState();

	Entity *addEntity(std::string name, PhysType physType, Model *model,
					  Transform *transform);
	// void removeEntity(Entity* e);  <-- probably not necessary, might help
	// optimize the game if we simply stop rendering certain entities instead.
	Entity *findEntity(std::string name);
	void endGame(Entity *gameWinner);
	void resetGameState();

	// Entity tracking
	std::vector<Entity> entityList;
	Entity *winner;

	// Flags
	bool gameEnded = false;

	// Game Parameters
	int numPlayers = 1;
	int numSparks = 8;
	int numActivePlayers = 8;
	int numLaps = 3;
};
