// does all the managing of the laps
// this includes creating checkpoints based off the curve
#pragma once
#include <string>
#include "../ecs/System.h"
#include "../ecs/Coordinator.h"
#include "../ecs/Component.h"
#include "../GameState.h"
#include "CurveLoader.h"


// Lap component
struct LapCounter {
	int lastCheckpointID = 0; // used for detecting the next valid checkpoint
	int currentLap = 1; // used for which lap the entity is currently on (player will pass this to UI)
	float progress = 0.0f; // progress along the track curve (measured by distance of the line segments)
	glm::vec3 lastCheckpointPos = {0.0f, 0.0f, 0.0f}; // used for keeping track of the position of the last checkpoint
};


class LapSystem : public System {
public:
	static std::shared_ptr<LapSystem> registerSystem(std::shared_ptr<Coordinator>& coord); // ecs shenanigans
	void generateCheckpoints(const std::string trackPath); // stores the checkpoints internally
	void update(GameState& game); // does the logic, i feel like it doesn't have to be every frame



private:
	std::vector<glm::vec3> checkPoints; // will need to be more sophisticated for multiple branching paths
	std::vector<TrackCurve> trackPoints; // vector of all track points
	float trackDistance; // length of the track curve
	float skipThresholdRatio = 0.1f; // How much of the track you can skip (0-1.0f)
	

};

