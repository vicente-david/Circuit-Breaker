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
	std::vector<Entity> standings; // vector of entities in order of first to second (standings[0] = 1st place)
};

class LeaderboardSystem : public System {
public:
	static std::shared_ptr<LeaderboardSystem> registerSystem(std::shared_ptr<Coordinator>& coord);
	void update(GameState& game); // does the respawn logic, probably safe to check every frame

private:
	std::vector<std::pair<float, Entity>> scores; // vector of pairs to keep track of each entity's "score".. easiest way to keep track of total progress for each entity
};