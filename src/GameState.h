#pragma once

#include "InputSystem.h"
#include "audio/AudioEngine.h"
#include "ecs/Coordinator.h"
#include "ecs/EntityManager.h"
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

	void endGame(EcsEntity gameWinner);
	void resetGameState();

	EcsEntity winner = -1;

	// Flags
	bool gameEnded = false;

	// Game Parameters
	int numPlayers = 1;
	int numSparks = 8;
	int numActivePlayers = 8;
	int numLaps = 3;
};
