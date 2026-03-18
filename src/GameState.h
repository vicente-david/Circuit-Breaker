#pragma once
#include "InputSystem.h"
#include "audio/AudioEngine.h"
#include "ecs/Coordinator.h"
#include "ecs/EntityManager.h"
#include "physics/PhysicsManager.h"
#include "ui/UISystemComponents.h"
#include "ui/UISystem.h"

#include <memory>

class GameState {
  public:
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

	//temporary to make everything work
	std::shared_ptr<Coordinator> coordinator;
	std::shared_ptr<UISystem> uiSystem;
	std::shared_ptr<PhysicsManager> physics;
	std::shared_ptr<AudioEngine> audio;
};
