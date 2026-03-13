// helper class to cleanup code

// will contain all the subsystems

// gamestate will purely hold the state of the game

// this Game class will talk between the subsystems

// it will pass around the game state as means of communication
#pragma once
#include "AllComponents.h"
#include "AllSystem.h"
#include "ecs/Coordinator.h"
#include "world/Track.h"

class Game{

public:
	Game();

	// general managers that things will often need to play sounds, add
	// entities, etc.
	std::shared_ptr<Coordinator> coordinator;
	std::shared_ptr<PhysicsManager> physics;
	std::shared_ptr<UISystem> uiSystem; // to-do: make a manager
	std::shared_ptr<AudioEngine> audio;

	void initializeGame();
	void initializeECS();
	void initializeTrack();
	void initializeFinishLine(); // ???

	// shared pointers to all the existing systems
	std::shared_ptr<PhysicsSystem> physicsSys;
	std::shared_ptr<RenderingSystem> renderer;
	std::shared_ptr<SparkSys> sparkSys;
	std::shared_ptr<ControllerSys> controllerSys;
	std::shared_ptr<CameraSystem> cameraSys;
	std::shared_ptr<AudioSystem> audioSys;
	std::shared_ptr<AIControllerSys> aiControllerSys;
	std::shared_ptr<RespawnSystem> respawnSys;
	std::shared_ptr<LapSystem> lapSys;
	std::shared_ptr<UISystem> uiSys;

	GameState gameState;
};