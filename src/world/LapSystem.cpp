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
#include <glm/gtc/integer.hpp>


// QOL functions

// sdf of a sphere
// returns distance to a sphere where p is the test point and r is radius of sphere
// will use this for testing collision (temporarily until we get physx)
// if the distance is negative that means inside the sphere
float sphereSDF(glm::vec3 p, float r) {
	return glm::length(p) - r;
}

// used for finding the next checkpoints reachable by an entity
// used for limiting which checkpoints to test for
int LapSystem::nearestCheckpoints(LapCounter& lapProg) {
	// the first part determines the search window for the projection
	float checkpointBounds = skipThresholdRatio * trackDistance; // next checkpoint must fall within this distance
	int nextCheckpoints = 1; // count of how many checkpoints ahead of the current checkpoint are allowed
	int startIndex = lapProg.lastCheckpointID + 1; // start index/id of the next checkpoint

	if (startIndex >= checkPoints.size()) startIndex = 0; // if index exceeds capacity, reset to 0

	int indexCP = checkpointPlacement * (startIndex); // index of checkpoint in trackPoints and trackDistances

	float startDistance = trackDistances[indexCP]; // start checking checkpoints from here
	float endDistance = startDistance + checkpointBounds; // stop checking checkpoints past this distance

	// find which checkpoints to check
	for (int i = startIndex; i < checkPoints.size(); i++) {

		if (trackDistances[i * checkpointPlacement] < endDistance) {
			nextCheckpoints++;
		}
		else {
			break; // early exit
		}
	}

	return nextCheckpoints;
}



// potential optimization by precomputing the length of the line segments
// given a line segment b-a (from a to b), return the closest point (and where the point is along the segment)
std::pair<glm::vec3, float> closestPoint(glm::vec3 a, glm::vec3 b, glm::vec3 p) {
	glm::vec3 lineSeg = b - a; // start at a, end at b
	glm::vec3 pointVec = p - a; // start at a, end at p
	float l2 = glm::dot(lineSeg, lineSeg); // length of the line segment squared
	float dotP = glm::dot(lineSeg, pointVec); // numerator of projection

	if (l2 == 0.0) return std::make_pair(a, 0.0f);

	// scalar along the line
	// if < 0 then we know it's behind a
	// if > 1 then we know it's past b
	// otherwise it will be in between 0,1
	// it just means how far along the line segment you are, 1 meaning b, 0 meaning a
	float t = dotP / l2; 

	return std::make_pair(a+t*lineSeg, t);


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
	
	trackDistances.push_back(0.0f); // cumulative distancce along track of the start line

	// for every point
	for (int i = 0; i < trackPoints[0].curvePoints.size()-1; i++) {
		// say every 10 paces is a checkpoint
		if (i % checkpointPlacement == 0) {
			// add those to our track checkpoints
			checkPoints.push_back(trackPoints[0].curvePoints[i]);
		}
		// for every line segment, add that to our track distance
		trackDistance += glm::length(trackPoints[0].curvePoints[i] - trackPoints[0].curvePoints[i+1]);
		trackDistances.push_back(trackDistance);
	}

	// start calculating the track distance (start-end) segment should be added (will not be added in the loop)
	trackDistance += glm::length(trackPoints[0].curvePoints.back() - trackPoints[0].curvePoints.front()); // end - start length
	std::cout << trackDistances.size() << std::endl;
	std::cout << trackPoints[0].curvePoints.size() << std::endl;
	for (int i = 0; i < trackPoints[0].curvePoints.size(); i++) {
		std::cout << trackPoints[0].curvePoints[i].x << " " << trackPoints[0].curvePoints[i].y << " "
			<< trackPoints[0].curvePoints[i].z << " with distance along track: " << trackDistances[i] << std::endl;
	}

}

// authoritative over progress (meaning that checkpoints will
// ultimately decide lap completion and stuff and not progressupdate)
// sequential update as well, you cannot skip checkpoints
// next checkpoints defines the range of checkpoints to check collision for
void LapSystem::updateCheckpoints(LapCounter& lapProg, Transform& eTransform, int nextCheckpoints, GameState& game, const Entity& entity) {
	for (int i = 0; i < nextCheckpoints; i++) {
	// more sophisticated if we want shortcuts, this is where the distance comes into play
	// track the forward progress the player has made on the track
	// and then if that is within an allowable range, let them reach the next checkpoint
	int indexI = lapProg.lastCheckpointID + i;

	// if out of bounds, then we want to cross the finish line
		if (indexI >= checkPoints.size()) {
			indexI = 0;
		}

		// check if vehicle is inside the next checkpoint
		if (sphereSDF(checkPoints[indexI] - eTransform.pos, 15.0f) < 0.0f) {
			// if they are check if their target isn't the start/finish line
			// the > 0 check is to make sure they don't immediately increment lap on race start
			if (lapProg.lastCheckpointID > 0 && indexI == 0) {
				lapProg.currentLap++;
				lapProg.progress = 0.0f;
				lapProg.closestTrackPoint = 0;
				//if (lapProg.isPlayer && !game.gameEnded) game.uiText = game.uiSystem->raceUI(lapProg.currentLap);
				std::cout << "on lap: " << lapProg.currentLap << std::endl;
				if (lapProg.currentLap >= 4 && !game.gameEnded) game.endGame(entity);
			}

			//std::cout << "checkpoint: " << indexI << std::endl;
			// update the vehicle checkpoint
			lapProg.lastCheckpointID = indexI;
			break;
		}
	}

}

// updates both checkpoint and distnace progressed along the track
void LapSystem::updateCheckpointsWithProgress(LapCounter& lapProg, Transform& eTransform, GameState& game, const Entity& entity) {

	int nextCheckpoints = LapSystem::nearestCheckpoints(lapProg);

	// now we have our search space for forward checkpoints

	int trackSize = trackPoints[0].curvePoints.size(); // size of the track QOL variable

	// find closest track segment, we'll limit our search space to 2 adjacent track segments
	int startIndex = lapProg.closestTrackPoint - 2; // start of search for projection
	// clamp the start index to start from the start line
	startIndex = glm::max(startIndex, 0);
	int endIndex = lapProg.closestTrackPoint + 3; // end of search for projection
	// clamp the end index to the finish line if overflow
	endIndex = glm::min(endIndex, trackSize); 

	int newClosestIndex = lapProg.closestTrackPoint;
	int newProg = lapProg.progress;

	float closestDistSquared = FLT_MAX; 

	for (int i = startIndex; i < endIndex-1; i++) {
		std::pair<glm::vec3, float> result = closestPoint(trackPoints[0].curvePoints[i], trackPoints[0].curvePoints[(i+1) % trackSize], eTransform.pos);
		if (result.second < 0.0 || result.second > 1.0) continue; // just means the player position is not closest to this segment

		// if the player position is closest to this segment, then update both progress and closest segment
		float newDistSquared = glm::dot(result.first-eTransform.pos, result.first - eTransform.pos);// player distance to projected point

		if (newDistSquared < closestDistSquared) {
			newClosestIndex = i;
			newProg = trackDistances[i] + result.second * (trackDistances[(i + 1) % trackSize] - trackDistances[i]);
			closestDistSquared = newDistSquared;
		}

		

	}

	// find the last reachable track segment, clamp it maximally to the last track segment

	int lastCPReach = lapProg.lastCheckpointID + nextCheckpoints;// index of furthest checkpoint reachable
	int lastSegment;

	// if it's into the next lap, then just don't check past the finish line
	if (lastCPReach >= checkPoints.size()) {
		lastSegment = trackPoints[0].curvePoints.size() - 1; // starts here and wraps into the first point
	}
	else {
		// otherwise if it's valid, just set it to the point behind the checkpoint
		lastSegment = glm::max(lastCPReach * checkpointPlacement - 1, 0);

	}

	/*
	float deltaDist = newProg - lapProg.progress; // detect forward or backward

	if (deltaDist < 0.0) {
		//std::cout << "backwards oml" << std::endl;
	}
	*/
	
	// upper limit
	if (newProg > trackDistances[lastSegment]) newProg = trackDistances[lastSegment];

	lapProg.progress = newProg;
	lapProg.closestTrackPoint = newClosestIndex;
		



	// updateCheckpoints
	LapSystem::updateCheckpoints(lapProg, eTransform, nextCheckpoints, game, entity);

//	std::cout << (lapProg.progress / trackDistance)*100 << "% complete" << std::endl;


}

// update
void LapSystem::update(GameState& game) {

	// for every entity with a lapcounter component and a transformcomponent
	for (auto& entity : entities) {
		LapCounter& lapProg = game.coordinator->getComponent<LapCounter>(entity);
		Transform& eTransform = game.coordinator->getComponent<Transform>(entity);

		updateCheckpointsWithProgress(lapProg, eTransform, game, entity);
		lapProg.lastCheckpointPos = checkPoints[lapProg.lastCheckpointID];
		lapProg.distToCheckpoint = glm::length(checkPoints[lapProg.lastCheckpointID] - eTransform.pos);

		// compute the track forward direction at this checkpoint used for respawning
			// use the vector from this checkpoint to the next one
		int followingCheckpoint = lapProg.lastCheckpointID + 1;
		if (followingCheckpoint >= checkPoints.size()) {
			followingCheckpoint = 0; // wrap around for the last checkpoint
		}
		glm::vec3 dir = checkPoints[followingCheckpoint] - checkPoints[lapProg.lastCheckpointID];
		dir.y = 0.0f; // project onto XZ plane so the vehicle stays upright
		if (glm::length(dir) > 0.001f) {
			lapProg.lastCheckpointDir = glm::normalize(dir);
		}
		

	}
}