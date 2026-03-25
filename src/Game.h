// helper class to cleanup code

// will contain all the subsystems

// gamestate will purely be updating the state of the game
// it could hold some states that are necessary
// gamestate will just be a channel where systems can talk to other systems

// it will pass around the game state as means of communication
#pragma once
#include "AllComponents.h"
#include "AllSystem.h"
#include "ecs/Coordinator.h"
#include "world/Track.h"
#include "InputSystem.h"

class Game{

public:
	Game();

	// general managers that things will often need to play sounds, add
	// entities, etc.
	std::shared_ptr<Coordinator> coordinator;
	std::shared_ptr<PhysicsManager> physics;
	std::shared_ptr<UISystem> uiSystem; // to-do: make a manager
	std::shared_ptr<AudioEngine> audio;

	// currently these are pretty disconnected
	// so make it so we can call initializeGame()

	// in theory this should initialize all internal systems
	// should not do any track loading or spawning of entities
	void initializeGame();

	// does all the track loading and spawning (should only run after menu)
	void initializeRace();
	void initializeECS();
	void initializeTrack();
	void initializeFinishLine(); // ???
	void initializeAudio();
	void initializeUI();

	//void initializePlayers();
	void initializePlayerSpark(std::vector<TrackCurve>& trackPaths, glm::vec3 pathStartPt); // initializes player spark
	void initializeAISpark(std::vector<TrackCurve>& trackPaths, glm::vec3 pathStartPt, std::string name); // ai spark
	void initializeAISpark2(std::vector<TrackCurve>& trackPaths, glm::vec3 pathStartPt); // to-do join above line together 


	//void resetGame(); // todo: implement

	void stateTransition(); // if gamestate changes update things
	// mostly a convenience function

	// update every frame 
	void update(); // calls the various updates, ex first update physics then rendering
	void updateTime(); 
	void updatePhysics();
	void updateFPS();
	void updateRendering();
	void updateAudio();

	// update not every frame (used for things that don't need to be updated every frame)

	// shared pointers to all the existing systems
	std::shared_ptr<PhysicsSystem> physicsSys;
	std::shared_ptr<RenderingSystem> renderer;
	std::shared_ptr<SparkSys> sparkSys;
	std::shared_ptr<ControllerSys> controllerSys;
	std::shared_ptr<CameraSystem> cameraSys;
	std::shared_ptr<AudioSystem> audioSys;
	std::shared_ptr<AIControllerSys> aiControllerSys;
	std::shared_ptr<RespawnSystem> respawnSys;
	std::shared_ptr<LeaderboardSystem> leaderboardSys;
	std::shared_ptr<LapSystem> lapSys;
	std::shared_ptr<UISystem> uiSys;

	InputSystem inputSystem;
	GameState gameState;

	Actions gameActions;

	bool isInitialized;


	// time stuff
	double t = 0.0;
	const double dt = 1.0 / 60.0; // simulate at 60fps
	// if its slower than this, just slow down the game instead of lagging even
	// more
	const double minFps = 30.0;
	double currentTime = glfwGetTime();
	double accumulator = 0.0;
	int framesPassed = 0;
	std::string fps = std::to_string(0);

	// sound
	Sound testSound;

	Entity player; // player LOL
	//Track activeTrack;
};
