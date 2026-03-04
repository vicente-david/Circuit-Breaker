#pragma once
#include <glm/glm.hpp>

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

	glm::vec3 targetPos = glm::vec3(0.0f); // where the AI is trying to drive to
	float arrivalRadius = 1.0f; // how close a spark needs to be from the targetPos to consider it as "arrived"
	float steeringSharpness = 2.0f; // how aggressively the AI turns. 1.0 = 90 degrees. higher = 'snappier' turns
	float brakeDistance = 2.0f; // how close the AI needs to be from the targetPos to begin braking
};
