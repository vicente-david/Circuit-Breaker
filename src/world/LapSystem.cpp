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


// sdf of a sphere
// returns distance to a sphere where p is the test point and r is radius of sphere
// will use this for testing collision (temporarily until we get physx)
// if the distance is negative that means inside the sphere
float sphereSDF(glm::vec3 p, float r) {
	return glm::length(p) - r;
}

// a mod b
int modInt(int a, int b) {
	if (a % b < 0) return b + (a % b);
	else return a % b;
}


// given a line segment b-a (from a to b), return the closest point (and where the point is along the segment)
// returns a if p gets projected behind a
std::pair<glm::vec3, float> closestPoint(glm::vec3 a, glm::vec3 b, glm::vec3 p) {
	glm::vec3 lineSeg = b - a; // start at a, end at b
	glm::vec3 pointVec = p - a; // start at a, end at p
	float l2 = glm::dot(lineSeg, lineSeg); // length of the line segment squared
	float dotP = glm::dot(lineSeg, pointVec); // numerator of projection

	if (dotP < 0.0 || l2 == 0.0) return std::make_pair(a, 0.0f);

	// scalar along the line
	// if > 1 then map to b
	// otherwise it will be in between 0,1
	// it just means how far along the line segment you are, 1 meaning b, 0 meaning a
	float t = dotP / l2; 

	if (t >= 1) return std::make_pair(b, 1.0);
	else return std::make_pair(a + t * lineSeg, t);


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
void LapSystem::updateCheckpoints(LapCounter& lapProg, Transform& eTransform) {
	// check which should be the next checkpoint, we'll assume for now you can't skip any checkpoints
	int nextCheckpoint = lapProg.lastCheckpointID + 1;

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
			lapProg.progress = 0.0f;
			std::cout << "on lap: " << lapProg.currentLap << std::endl;
		}

		std::cout << "checkpoint: " << nextCheckpoint << std::endl;
		// update the vehicle checkpoint
		lapProg.lastCheckpointID = nextCheckpoint;

	}
}

// updates both checkpoint and distnace progressed along the track
void LapSystem::updateCheckpointsWithProgress(LapCounter& lapProg, Transform& eTransform) {
	
	// the first part determines the search window for the projection
	float checkpointBounds = skipThresholdRatio* trackDistance; // next checkpoint must fall within this distance
	int nextCheckpoints = 0; // count of how many checkpoints ahead of the current checkpoint are allowed
	int startIndex = lapProg.lastCheckpointID + 1; // start index/id of the next checkpoint

	if (startIndex >= checkPoints.size()) startIndex = 0; // if index exceeds capacity, reset to 0

	int indexCP = checkpointPlacement * (startIndex); // index of checkpoint in trackPoints and trackDistances
	
	float startDistance = trackDistances[indexCP]; // start checking checkpoints from here
	float endDistance = startDistance + checkpointBounds; // stop checking checkpoints past this distance

	// find which checkpoints to check
	for (int i = startIndex; i < checkPoints.size(); i++) {
		
		if (trackDistances[i*checkpointPlacement] < endDistance) {
			nextCheckpoints++;
		}
		else {
			break; // early exit
		}
	}

	// now we have our search space

	// our next step is to find the closest track segment to the player
	// we have our search window, we check that many back, and that many forward

	int closeIndexS = (lapProg.lastCheckpointID - nextCheckpoints); // start of search
	int closeIndexF = (lapProg.lastCheckpointID + nextCheckpoints); // end of search
	// this could take vectors that are out of bounds, so when we do our accessing, we use modulus

	int trackSize = trackPoints[0].curvePoints.size(); // size of the track QOL variable

	closeIndexS = closeIndexS * checkpointPlacement; 
	// map it to the track point we wish to start from
	closeIndexF = closeIndexF * checkpointPlacement;

	std::cout << modInt(closeIndexS, trackSize) << std::endl;
	
	int closestSegment = modInt(closeIndexS, trackSize); // index of the start point of the closest track segment to the player
	float progDist = trackDistances[closestSegment];
	float minDist = glm::dot(eTransform.pos - trackPoints[0].curvePoints[closestSegment], eTransform.pos - trackPoints[0].curvePoints[closestSegment]); // minimum squared distance found so far 
	

	for (int i = closeIndexS; i < closeIndexF-1; i++) {
		// construct our vectors
		glm::vec3 A = trackPoints[0].curvePoints[modInt(i, trackSize)]; // start of track segment 
		// apparently python and c++ % are different for negative numbers
		glm::vec3 B = trackPoints[0].curvePoints[modInt(i+1, trackSize)]; // end of track segment

		// first component has the closest point on the line segment
		// second component has how far along the line segment
		std::pair<glm::vec3, float> result = closestPoint(A, B, eTransform.pos);

		// distance along line segment
		float distOnSeg = trackDistances[modInt(i+1, trackSize)] - trackDistances[modInt(i, trackSize)];
		distOnSeg = distOnSeg * result.second;

		// we only want to update forward progress so if the above distance isn't posiitve, just continue through the loop
		if (trackDistances[modInt(i, trackDistances.size())] + distOnSeg < lapProg.progress) continue;

		glm::vec3 P = result.first - eTransform.pos; // vector of closest point on segment to player
		float dotP = glm::dot(P, P);
		
		// we can optimize by comparing squared distances
		if (dotP < minDist) {
			minDist = dotP;
			closestSegment = modInt(i, trackPoints[0].curvePoints.size());
			progDist = trackDistances[modInt(i, trackDistances.size())] + distOnSeg;
		}

	}

	lapProg.progress = progDist; //update only forward progress
	
	std::cout << (progDist / trackDistance)*100 << "% complete" << std::endl;


}

// constantly update lap progress 
void LapSystem::updateProgress(LapCounter& lapProg, Transform& eTransform) {
	// we update the progress by projecting the player position onto the closest line segment

	// the trouble is how do we know which is the closest track segment?
	// well we can always loop through all of them in the worst case scenario (pretty bad for performance)

	// so instead we'll limit the search space
	// we have a ratio of how much of the track we can skip at a time
	
	// so we just check from the current checkpoint in two directions (forward and back) by the ratio
	// take the minimum distance of that and we have found the closest track segment

	// a glaring issue with this approach is we never update checkpoints when going in reverse
	// so if a player starts driving backwards, then should the progress update or not?

	
}

// update
void LapSystem::update(GameState& game) {

	// for every entity with a lapcounter component and a transformcomponent
	for (auto& entity : entities) {
		LapCounter& lapProg = game.coordinator->getComponent<LapCounter>(entity);
		Transform& eTransform = game.coordinator->getComponent<Transform>(entity);

		updateCheckpoints(lapProg, eTransform);
		updateProgress(lapProg, eTransform);
		updateCheckpointsWithProgress(lapProg, eTransform);

	}
}