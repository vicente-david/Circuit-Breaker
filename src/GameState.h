#pragma once
#include "InputSystem.h"
#include "audio/AudioEngine.h"
#include "ecs/Coordinator.h"
#include "ecs/EntityManager.h"
#include "physics/PhysicsManager.h"

#include <memory>
#include <functional>
#include <vector>
#include <string>

class UISystem; // forward declaration (UISystem.h includes GameState.h, so we can't include it here)

enum GAMESTATE {
	MAINMENU,
	GAMEPLAY,
	PAUSED,
	END,
	TUTORIAL,
};


class GameState {
public:
	Actions inputActions;
	UIActions uiActions;

	GAMESTATE currentState = MAINMENU; // stores the current state of the game
	GAMESTATE nextState = MAINMENU; // used for handling state transitions

	// Public functions
	GameState();

	void endGame(Entity gameWinner);
	void resetGameState();

	// i think these need to be moved up, setting inputs can stay here but the system management
	// may need to be managed by Game.cpp and not gameState

	/*
	void setGame(); // wrapper to ready all systems for playing game

	void setTitleRenderPasses(); // set render passes for title screen
	void setGameRenderPasses(); // set render passes for the actual game (when playing)
	void setPauseRenderPasses(); // pause menu

	void setTitleInputs(); // swap actions to title menu
	void setGameInputs(); // swap actions to in game actions
	void setPauseInputs(); // swaps to pause inputs
	*/

	Entity winner = -1;

	// finish order: names pushed in the order that players finished (i.e. finishOrder[0] = 1st place
	std::vector<std::string> finishOrder;

	// Flags
	bool gameEnded = false;
	bool raceStart = true;
	bool countdownActive = false;

	// Game Parameters
	int numPlayers = 1;
	int numSparks = 8;
	int numActivePlayers = 8;
	int numLaps = 4;

	double* frameTime;

	// temp UI
	//RectUI activeUIRect; 
	//TextUI uiText;
	Entity* player;
	bool playerBackwards = false; // detect when the player is backwards

	//temporary to make everything work
	std::shared_ptr<Coordinator> coordinator;
	std::shared_ptr<UISystem> uiSystem;
	std::shared_ptr<PhysicsManager> physics;
	std::shared_ptr<AudioEngine> audio;

};
