#pragma once

#include "InputSystem.h"
#pragma once
#include "audio/AudioEngine.h"
#include "ecs/Coordinator.h"
#include "ecs/EntityManager.h"
#include "physics/PhysicsManager.h"
#include "ui/UISystemComponents.h"
#include "ui/UISystem.h"

#include <memory>

class GameState {
  public:
	// general managers that things will often need to play sounds, add
	// entities, etc.
	std::shared_ptr<Coordinator> coordinator;
	std::shared_ptr<PhysicsManager> physics;
	std::shared_ptr<UISystem> uiSystem; // to-do: make a manager
	std::unique_ptr<AudioEngine> audio;
	Actions inputActions;

	// Public functions
	GameState();

	void endGame(Entity gameWinner);
	void resetGameState();

	Entity winner = -1;

	// Flags
	bool gameEnded = false;
	bool raceStart = true;

	// Game Parameters
	int numPlayers = 1;
	int numSparks = 8;
	int numActivePlayers = 8;
	int numLaps = 4;

	// temp UI
	RectUI activeUIRect; 
	TextUI uiText;

};
