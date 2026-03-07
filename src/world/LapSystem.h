// does all the managing of the laps
// this includes creating checkpoints based off the curve
#pragma once
#include <string>
#include "../ecs/System.h"
#include "../ecs/Coordinator.h"
#include "../ecs/Component.h"
#include "../GameState.h"
#include "CurveLoader.h"
#include <glm/gtx/projection.hpp>




// Lap component
struct LapCounter {
	int lastCheckpointID = 0; // used for detecting the next valid checkpoint
	int currentLap = 1; // used for which lap the entity is currently on (player will pass this to UI)
	float progress = 0.0f; // progress along the track curve (measured by distance of the line segments)
	int closestTrackPoint = 0; //index of the closest track point, used to limit the search space for the projection
};


class LapSystem : public System {
public:
	static std::shared_ptr<LapSystem> registerSystem(std::shared_ptr<Coordinator>& coord); // ecs shenanigans
	void generateCheckpoints(const std::string trackPath); // stores the checkpoints internally
	void update(GameState& game); // does the logic, i feel like it doesn't have to be every frame



private:
	int nearestCheckpoints(LapCounter& lapProg); // find the next checkpoints to test for

	void updateCheckpoints(LapCounter& lapProg, Transform& eTransform, int nextCheckpoints); // update the checkpoint for the entity
	void updateCheckpointsWithProgress(LapCounter& lapProg, Transform& eTransform); // update the checkpoint for the entity
	void updateProgress(LapCounter& lapProg, Transform& eTransform); // update the progress along the track
	
	std::vector<glm::vec3> checkPoints; // will need to be more sophisticated for multiple branching paths
	int checkpointPlacement = 10; // every x amount of points along the track, place 1 checkpoint

	std::vector<TrackCurve> trackPoints; // vector of all track points
	std::vector<float> trackDistances; // tracks cumulative distance along the track (same order as the trackPoints)
	float trackDistance; // length of the track curve
	float skipThresholdRatio = 0.1f; // How much of the track you can skip (0-1.0f)
	

};

