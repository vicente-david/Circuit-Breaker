// Communicates with a higher level to determine lap count
// Depends on spark position
// will need to store all entities and their associated checkpoints/laps
// but guess what, we have an ecs system, we'll just have it store all entities
// have a component called like Lap
// and do it based off of that

// if we have a bunch of line segments that connect to one another, what is a good way to make checkpoints?
// don't remove points
// an idea is to use distances
// first calculate full track distance using the line segments
// then trim the track points to check points
// each checkpoint will have a distance threshold assigned to it (as collision)
// (this distance could actually be signed)
// benefit of signed distance is you could do crazy shortcuts that skip things, but if you go backward
// your distance will have opposite sign

// the above seems like a pain, we could just use thresholds, if your jump along the spline is a valid threshhold, then you're ok
#pragma once
#include "LapSystem.h"
#include <iostream>
#include "CurveLoader.h"


// sdf of a sphere
// returns distance to a sphere where p is the test point and r is radius of sphere
// will use this for testing collision (temporarily until we get physx)
// if the distance is negative that means inside the sphere
float sphereSDF(glm::vec3 p, float r) {
	return glm::length(p) - r;
}

std::shared_ptr<LapSystem> LapSystem::registerSystem(std::shared_ptr<Coordinator>& coord) {
	// register system
	auto system = coord->registerSystem<LapSystem>();
	// create system signture (what components this system needs)
	Signature sig;
	sig.set(coord->getComponentType<Transform>());
	sig.set(coord->getComponentType<LapCounter>());
	
	coord->setSystemSignature<LapSystem>(sig);

	return system;
}

// generate checkpoints
void LapSystem::generateCheckpoints(const std::string path) {
	// load track curve (could be multiple, as of right now only one)
	// will need to be more sophisticated later when we add branches
	trackPoints = CurveLoader::loadCurve(path);

	// start calculating the track distance (start-end) segment should be added (will not be added in the loop)
	trackDistance = glm::length(trackPoints[0].curvePoints.back() - trackPoints[0].curvePoints.front()); // end - start length
	
	// for every point
	for (int i = 0; i < trackPoints[0].curvePoints.size()-1; i++) {
		// say every 10 paces is a checkpoint
		if (i % 10 == 0) {
			// add those to our track checkpoints
			checkPoints.push_back(trackPoints[0].curvePoints[i]);
		}
		// for every line segment, add that to our track distance
		trackDistance += glm::length(trackPoints[0].curvePoints[i] - trackPoints[0].curvePoints[i+1]);
	}
}

// update
void LapSystem::update(GameState& game) {

	// for every entity with a lapcounter component and a transformcomponent
	for (auto& entity : entities) {
		LapCounter& lapProg = game.coordinator->getComponent<LapCounter>(entity);
		Transform& eTransform = game.coordinator->getComponent<Transform>(entity);

		// check which should be the next checkpoint, we'll assume for now you can't skip any checkpoints
		int nextCheckpoint = lapProg.lastCheckpointID+1;

		// more sophisticated if we want shortcuts, this is where the distance comes into play
		// track the forward progress the player has made on the track
		// and then if that is within an allowable range, let them reach the next checkpoint

		// if out of bounds, then we want to cross the finish line
		if (nextCheckpoint >= checkPoints.size()) {
			nextCheckpoint = 0;
		}

		// check if vehicle is inside the next checkpoint
		if (sphereSDF(checkPoints[nextCheckpoint] - eTransform.pos, 10.0f) < 0.0f) {
			// if they are check if their target isn't the start/finish line
			// the > 0 check is to make sure they don't immediately increment lap on race start
			if (lapProg.lastCheckpointID > 0 && nextCheckpoint == 0) {
				lapProg.currentLap++;
				std::cout << "on lap: " << lapProg.currentLap << std::endl;
			}

			std::cout << "checkpoint: " << nextCheckpoint << std::endl;
			// update the vehicle checkpoint
			lapProg.lastCheckpointID = nextCheckpoint;
			
			lapProg.lastCheckpointPos = checkPoints[lapProg.lastCheckpointID];
		}

	}
}