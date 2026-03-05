#pragma once
#include <glm/glm.hpp>
#include "../world/CurveLoader.h"

// FSM states for AI sparks
enum AIState {
	IDLE = 0,
	DRIVING = 1,
	BRAKING = 2,
	DRIFTING = 3,
	BOOSTING = 4,
	ATTACKING = 5
};

// this component tells the controller system that its AI controlled.
struct AIController {
	AIState state = IDLE;

	std::vector<TrackCurve> paths{}; // set of paths along track
	std::vector<glm::vec3> route{}; // Current route plan for the ai
	int targetIdx = 50; // index of target position
	int currentPosIdx = 50; // approx index current position of spark
	
	// glm::vec3 targetPos = glm::vec3(0.0f); // where the AI is trying to drive to
	float arrivalRadius = 10.0f; // how close a spark needs to be from the targetPos to consider it as "arrived"
	float steeringSharpness = 2.0f; // how aggressively the AI turns. 1.0 = 90 degrees. higher = 'snappier' turns
	float brakeDistance = 2.0f; // how close the AI needs to be from the targetPos to begin braking
};
