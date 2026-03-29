// does all the managing of the laps
// this includes creating checkpoints based off the curve
#pragma once
#include <string>
#include "../ecs/System.h"
#include "../ecs/Coordinator.h"
#include "../ecs/Component.h"
#include "../GameState.h"
#include "CurveLoader.h"
#include "vehicles/SparkComponents.h"
#include <glm/gtx/projection.hpp>




// Lap component
struct LapCounter {
	int lastCheckpointID = 0; // used for detecting the next valid checkpoint
	int currentLap = 1; // used for which lap the entity is currently on (player will pass this to UI)
	float progress = 0.0f; // progress along the track curve (measured by distance of the line segments)
	int closestTrackPoint = 0; //index of the closest track point, used to limit the search space for the projection
	glm::vec3 lastCheckpointPos = { 0.0f, 0.0f, 0.0f }; // used for keeping track of the position of the last checkpoint
	glm::vec3 lastCheckpointDir = { 0.0f, 0.0f, 1.0f }; // forward direction of the track at the last checkpoint
	int lastCheckpointIdx = 0; // curve index of the position of the last checkpoint
	bool isPlayer = false;
	float distToCheckpoint = 0.0f; // used by the leaderboard system to decide who's closest to the next checkpoint
	
};



class LapSystem : public System {
public:
	static std::shared_ptr<LapSystem> registerSystem(std::shared_ptr<Coordinator>& coord); // ecs shenanigans
	void generateCheckpoints(const std::vector<TrackCurve> paths); // stores the checkpoints internally
	void update(GameState& game); // does the logic, i feel like it doesn't have to be every frame

	
private:
	int nearestCheckpoints(LapCounter& lapProg); // find the next checkpoints to test for

	void updateCheckpoints(LapCounter& lapProg, Transform& eTransform, int nextCheckpoints, GameState& game, const Entity& entity); // update the checkpoint for the entity
	void updateCheckpointsWithProgress(LapCounter& lapProg, Transform& eTransform, GameState& game, const Entity& entity); // update the checkpoint for the entity
	
	std::vector<int> checkpointIdx;
	int checkpointPlacement = 10; // every x amount of points along the track, place 1 checkpoint
	std::vector<std::pair<glm::vec3, glm::vec3>> checkPoints; // checkpoint has position on main path and position on 'heal' path
	std::vector<TrackCurve> trackPoints; // vector of all track points
	std::vector<float> trackDistances; // tracks cumulative distance along the track (same order as the trackPoints)
	float trackDistance; // length of the track curve
	float skipThresholdRatio = 0.10f; // How much of the track you can skip (0-1.0f)
	

};

