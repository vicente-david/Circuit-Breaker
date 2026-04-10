#pragma once
#include "../GameState.h"
#include "ecs/System.h"
#include "ecs/Coordinator.h"
#include "ecs/Component.h"
#include "LapSystem.h"
#include <utility> // for std::pair

// leaderboard component which stores the entity's current leaderbord position, and the full standings (might not need the full standings?)
struct Leaderboard {
	//int leaderboardPos = 0;
	std::vector<std::string> standings; // vehicle names in order of first to last (standings[0] = 1st place)
	std::vector<std::string> finalPositions; // final standings updated as each player finishes the race
};

class LeaderboardSystem : public System {
public:
	static std::shared_ptr<LeaderboardSystem> registerSystem(std::shared_ptr<Coordinator>& coord);
	void update(GameState& game); // does the respawn logic, probably safe to check every frame
	void reset(); // call when restarting a race to clear the locked final positions

private:
	std::vector<std::pair<float, std::string>> scores; // vector of pairs to keep track of each entity's "score".. easiest way to keep track of total progress for each entity
	bool finalPositionsLocked = false; // true once the final standings have been randomized and locked in
};